import { createAdsHandler } from './handler.mjs';
const secrets=JSON.parse(Deno.env.get('SUPABASE_SECRET_KEYS')||'{}');
Deno.serve(createAdsHandler({
 url:Deno.env.get('SUPABASE_URL')!,
 secret:Deno.env.get('SUPABASE_SERVICE_ROLE_KEY')||secrets.default,
 origin:'https://freemotra-sponsor-studio.sonatacello.chatgpt.site',
}));

