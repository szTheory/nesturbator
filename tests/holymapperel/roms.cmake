# The Holy Mapperel ROMs that tests/CMakeLists.txt, write_hashes.cmake and
# hash_inventory.cmake all read. Each entry is
# <key>:<file in tests/roms/hm>:<N>:<prg ram text>[:save]. N is the frame whose
# result screen is read and hashed. The prg ram text is the exact text of the
# screen's PRG RAM row (nametable row 6), asserted by holymapperel.<key>.prgram
# with holymapperel-decode --prg-ram. An entry ending in :save runs the two-run
# save chain that Plan 08-07 adds. Phases 8 and 9 add one entry here and one
# line to the manifest list below per ROM.
set(NESTURBATOR_HOLYMAPPEREL_ROMS
  "m2:M2_P128K_CR8K_V.nes:100:PRG RAM MISSING"
  "m3:M3_P32K_C32K_H.nes:100:PRG RAM MISSING"
  "m7:M7_P128K_CR8K.nes:100:PRG RAM MISSING")

# The manifest.txt line of each ROM, so write_hashes.cmake can check provenance.
set(NESTURBATOR_HOLYMAPPEREL_MANIFEST
  "tests/roms/hm/M2_P128K_CR8K_V.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\tc7e83755bd9adbb7c705ea9f29535442af7390444632f9f824fcf4c00632069b"
  "tests/roms/hm/M3_P32K_C32K_H.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t499891c6d8c7a1e7631bdc601d9d624938842735f92fe1fda7b19ef9fae514b7"
  "tests/roms/hm/M7_P128K_CR8K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t4aa0050f36ae17e17701506821e5147df5b843bcdf2827468fa9c66e4b7ac1ba")
