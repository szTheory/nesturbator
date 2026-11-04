# Phase 25: Region selection and PAL timing - Discussion Log

> **Audit trail only.** Decisions are captured in `25-CONTEXT.md`; this log preserves the alternatives considered.

**Date:** 2026-11-01  
**Phase:** 25-region-selection-and-pal-timing  
**Areas discussed:** Host region preference, PAL output height

---

## Host region preference

| Option | Description | Selected |
|--------|-------------|----------|
| Per-content load preference; host owns persistence | Pass `Auto/NTSC/PAL` on each load; host may persist per game; core has no preference store. | ✓ |
| One core-wide preference for all ambiguous content | A single core preference selects ambiguous/legacy content across titles. | |
| Different behavior | Owner may describe another policy. | |

**User's choice:** Per-content load preference; host owns persistence.
**Notes:** The effective region is fixed until unload/reload. Existing approved policy remains: explicit single-region NES 2.0 wins; ambiguous/legacy `Auto` defaults to NTSC; Dendy is refused.

---

## PAL output height

| Option | Description | Selected |
|--------|-------------|----------|
| Keep the 256×240 public output | Preserve the library/libretro contract; establish row-239 treatment from permitted evidence before visual assertions. | ✓ |
| Expose 256×239 for PAL | Change the public PAL buffer geometry. | |
| Different behavior | Owner may describe another policy. | |

**User's choice:** Keep the 256×240 public output.
**Notes:** Internal PAL raster timing remains region-specific. Do not guess row-239 pixel behavior or claim hardware verification.

---

## the agent's Discretion

- Choose concrete API representation and adapter option plumbing under the existing project patterns and locked behavior.
- Keep the implementation dependency-free and source/evidence claims within their documented limits.

## Deferred Ideas

- No new ideas were added. Approved Phase 26 and 27 work and unrelated deferred testing todos remain outside Phase 25.
