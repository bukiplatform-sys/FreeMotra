create table public.freemotra_relay_config (
 id boolean primary key default true check(id), enabled boolean not null default false,
 mode text not null default 'rest' check(mode in ('rest','static')),
 urls jsonb not null default '[]'::jsonb check(jsonb_typeof(urls)='array' and jsonb_array_length(urls)<=2),
 username text not null default '', secret_box text not null default '', updated_at timestamptz not null default now()
);
alter table public.freemotra_relay_config enable row level security;
revoke all on public.freemotra_relay_config from anon, authenticated;
grant all on public.freemotra_relay_config to service_role;
insert into public.freemotra_relay_config(id) values(true);
