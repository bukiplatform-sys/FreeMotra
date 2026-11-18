# FreeMotra 0.3.1 advertising connection

Supabase project: `mesqqxlacjuvinsdospy` (FreeMotra, separate from Buki).
Owner-private administration: https://freemotra-sponsor-studio.sonatacello.chatgpt.site

The Windows application now reads its default feed from Supabase REST using
the publishable read-only key in `src/SponsorConfig.h`. Only rows enabled by
the active flag and schedule pass the SELECT RLS policy. Drafts, administrator
password hashes, sessions and campaign editing data have no client access.
The service-role key exists only in the Supabase Edge Function environment;
it is never included in the browser, application or repository.

The management page calls `freemotra-ads` for login, upload and publishing.
Passwords use PBKDF2-SHA256 (310,000 iterations, random salt); server sessions
store only token hashes and expire after 30 minutes. Login attempts are
limited in the database. Changing the password revokes every admin session.
The initial password is delivered privately and is not included here.

Publishing replaces the complete campaign in one database transaction with
an expected revision, so simultaneous editors cannot silently overwrite each
other. PNG uploads are capped at 1 MB and 1600 x 1600 pixels. Uploaded ad
images are public immutable hash-named assets; do not upload confidential
files. Public users cannot upload, list or modify storage objects.

Clients refresh the campaign approximately every 15 minutes, outside remote
sessions. Public reads use REST directly, rather than consuming an Edge
Function invocation for each client poll. Images use a bounded local HTTP
disk cache. Version 0.3.0 has no default server; users must install 0.3.1 once.
No advertisement updates require another executable after that migration.

Canonical schema: `supabase/schema.sql`. The Edge Function intentionally uses
`verify_jwt=false`: it verifies its own opaque administrator sessions before
each protected request. Health is public; every administration route except
password login requires a valid session. Administrative calls accept only
the configured browser origin and explicit bearer tokens, not cookies.

Checks: `node --test sponsor-admin/test.mjs supabase/tests.mjs`, Windows
`sponsor_document` tests, and the private integration script
`supabase/verify-live.mjs` (administrator password through hidden stdin).
