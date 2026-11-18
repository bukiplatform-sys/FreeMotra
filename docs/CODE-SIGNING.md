# Code signing policy

FreeMotra currently publishes unsigned Windows executables. A self-signed
certificate would not establish a trusted Windows publisher and is not used.

CI produces GitHub/Sigstore build provenance for the release EXE and ZIP.
Users can verify the repository and digest with:

    gh attestation verify FreeMotra-0.4.0-windows-x64.exe --repo bukiplatform-sys/FreeMotra

Build provenance is separate from Windows Authenticode publisher signing.

## Free Authenticode path

SignPath Foundation accepts eligible open-source projects for free signing:
https://signpath.org/ and https://signpath.org/terms . Approval and release
signing policies must be arranged with the Foundation. FreeMotra has not
been accepted or endorsed; no application or message has been submitted.

Preparation included: public source and releases, product/version metadata,
license notices, CI checks and provenance, and the privacy description below.
The repository owner must confirm the project roles, apply, receive approval,
and configure the approved signing identity/policy. No signing credentials
are embedded in the application or committed to this repository.

Current repository maintainer: bukiplatform-sys. An independent reviewer and
release approver must be designated before claiming an independent audit.

## Privacy

Update checks contact GitHub. Advertisement retrieval contacts the configured
Supabase project. Internet invitations explicitly start Supabase Realtime
signaling, STUN discovery and the optional community TURN service. Signaling
contents are AES-256-GCM encrypted with the invitation key; WebRTC session
traffic uses DTLS. Providers can still observe connection metadata such as
IP addresses, room identifiers, timings and traffic sizes. LAN mode does not
use public signaling/ICE services. Microphone and remote control require
session permissions; microphone capture also requires the local toggle.
No automatic crash-report/content upload is implemented.

Supabase: https://supabase.com/privacy
GitHub: https://docs.github.com/site-policy/privacy-policies/github-general-privacy-statement
Cloudflare STUN: https://www.cloudflare.com/privacypolicy/
Open Relay: https://www.metered.ca/terms
