create table public.freemotra_admin_settings(id boolean primary key default true check(id),password_salt text not null,password_hash text not null,iterations integer not null default 310000 check(iterations>=310000));
create table public.freemotra_admin_sessions(token_hash text primary key,expires_at timestamptz not null);
create index freemotra_sessions_expiry on public.freemotra_admin_sessions(expires_at);
create table public.freemotra_login_limits(key text primary key,attempts integer not null,expires_at timestamptz not null);
create table public.freemotra_campaigns(id boolean primary key default true check(id),revision bigint not null default 0,sponsors jsonb not null default '[]'::jsonb check(jsonb_typeof(sponsors)='array' and jsonb_array_length(sponsors)<=20),updated_at timestamptz not null default now());
insert into public.freemotra_campaigns(id) values(true);
create table public.freemotra_ads(id text primary key,position integer not null,active boolean not null default false,starts_at timestamptz,ends_at timestamptz,payload jsonb not null check(jsonb_typeof(payload)='object'));
alter table public.freemotra_admin_settings enable row level security;
alter table public.freemotra_admin_sessions enable row level security;
alter table public.freemotra_login_limits enable row level security;
alter table public.freemotra_campaigns enable row level security;
alter table public.freemotra_ads enable row level security;
revoke all on public.freemotra_admin_settings,public.freemotra_admin_sessions,public.freemotra_login_limits,public.freemotra_campaigns,public.freemotra_ads from anon,authenticated;
grant all on public.freemotra_admin_settings,public.freemotra_admin_sessions,public.freemotra_login_limits,public.freemotra_campaigns,public.freemotra_ads to service_role;
grant select on public.freemotra_ads to anon,authenticated;
create policy freemotra_read_live_ads on public.freemotra_ads for select to anon,authenticated using(active and(starts_at is null or starts_at<=now())and(ends_at is null or ends_at>now()));
create function public.freemotra_publish(sponsor_list jsonb,expected_revision bigint) returns bigint language plpgsql security invoker set search_path='' as $$
declare next_revision bigint;
begin
 if jsonb_typeof(sponsor_list)<>'array' or jsonb_array_length(sponsor_list)>20 then raise exception 'invalid sponsor list'; end if;
 update public.freemotra_campaigns set sponsors=sponsor_list,revision=revision+1,updated_at=now() where id=true and revision=expected_revision returning revision into next_revision;
 if next_revision is null then raise exception 'revision conflict' using errcode='40001'; end if;
 delete from public.freemotra_ads where id is not null;
 insert into public.freemotra_ads(id,position,active,starts_at,ends_at,payload) select x.value->>'id',x.ordinality-1,(x.value->>'active')::boolean,nullif(x.value->>'start','')::timestamptz,nullif(x.value->>'end','')::timestamptz,x.value-'active'-'start'-'end' from jsonb_array_elements(sponsor_list) with ordinality as x(value,ordinality);
 return next_revision;
end; $$;
revoke all on function public.freemotra_publish(jsonb,bigint) from public,anon,authenticated;
grant execute on function public.freemotra_publish(jsonb,bigint) to service_role;
create function public.freemotra_login_attempt(attempt_key text) returns integer language plpgsql security invoker set search_path='' as $$
declare attempt_count integer;
begin
 delete from public.freemotra_login_limits where expires_at<now();
 insert into public.freemotra_login_limits(key,attempts,expires_at) values(attempt_key,1,now()+interval '15 minutes') on conflict(key) do update set attempts=public.freemotra_login_limits.attempts+1 returning attempts into attempt_count;
 return attempt_count;
end; $$;
revoke all on function public.freemotra_login_attempt(text) from public,anon,authenticated;
grant execute on function public.freemotra_login_attempt(text) to service_role;
insert into storage.buckets(id,name,public,file_size_limit,allowed_mime_types) values('freemotra-sponsor-images','freemotra-sponsor-images',true,1000000,array['image/png']);
