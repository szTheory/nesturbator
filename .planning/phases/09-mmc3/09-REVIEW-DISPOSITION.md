---
phase: 09-mmc3
review: 09-REVIEW.md
---

# Phase 9 review disposition

| Finding | Severity | Disposition | Note |
|---------|----------|-------------|------|
| WR-01 | warning | open | MMC3 powers on with vertical mirroring and ignores the header bit; the verifier checks whether D-07 intends this |
| IN-01 | info | open | Duplicate CHR pointer computation in map_chr_1k |
| IN-02 | info | open | describe_rejection repeats header decoding and the mapper list |
