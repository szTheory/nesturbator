# The Holy Mapperel ROMs that tests/CMakeLists.txt, write_hashes.cmake and
# hash_inventory.cmake all read. Each entry is
# <key>:<file in tests/roms/hm>:<N>:<prg ram text>[:save]. N is the frame whose
# result screen is read and hashed. The prg ram text is the exact text of the
# screen's PRG RAM row (nametable row 6), asserted by holymapperel.<key>.prgram
# with holymapperel-decode --prg-ram. An entry ending in :save runs the ROM
# twice against one save directory: run 1 starts empty and shows no BATTERY,
# run 2 loads the save run 1 wrote and shows "+ BATTERY". Phases 8 and 9 add
# one entry here and one line to the manifest list below per ROM.
set(NESTURBATOR_HOLYMAPPEREL_ROMS
  "m2:M2_P128K_CR8K_V.nes:100:PRG RAM MISSING"
  "m3:M3_P32K_C32K_H.nes:100:PRG RAM MISSING"
  "m7:M7_P128K_CR8K.nes:100:PRG RAM MISSING"
  "m4tnrom:M4_P128K_CR8K.nes:600:PRG RAM MISSING"
  "m1sgrom:M1_P128K_CR8K.nes:100:PRG RAM MISSING"
  "m1sjrom:M1_P128K_C32K_W8K.nes:100:8K PRG RAM OK"
  "m1skrom:M1_P128K_C128K_S8K.nes:100:8K PRG RAM OK"
  "m1surom:M1_P512K_CR8K_S8K.nes:200:8K PRG RAM OK"
  "m1sxrom:M1_P512K_CR8K_S32K.nes:400:32K PRG RAM OK:save")

# The manifest.txt line of each ROM, so write_hashes.cmake can check provenance.
set(NESTURBATOR_HOLYMAPPEREL_MANIFEST
  "tests/roms/hm/M1_P128K_CR8K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t0595f0896d71d6e54ad19b00a2cdfb6d1665e3166167a06935c5971105ed5793"
  "tests/roms/hm/M1_P128K_C32K_W8K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t3d330d32bb45d41ce6c9666b80f580ff2912444b44f833ee89d062794abdfe32"
  "tests/roms/hm/M1_P128K_C128K_S8K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t2263d212fe9c8e857f89c846962e7e78f3ba885e1413644b94e50cfff65ecd08"
  "tests/roms/hm/M1_P512K_CR8K_S8K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\tc3f239ea8f2fa1023a272b5e1b5add884b583e79ffd6487771cc53a1f4566b5c"
  "tests/roms/hm/M1_P512K_CR8K_S32K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t7db5b5191ce842d44a5a8cb132a58d7116741053d4bd0f8a5def832be80e655a"
  "tests/roms/hm/M2_P128K_CR8K_V.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\tc7e83755bd9adbb7c705ea9f29535442af7390444632f9f824fcf4c00632069b"
  "tests/roms/hm/M3_P32K_C32K_H.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t499891c6d8c7a1e7631bdc601d9d624938842735f92fe1fda7b19ef9fae514b7"
  "tests/roms/hm/M7_P128K_CR8K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\t4aa0050f36ae17e17701506821e5147df5b843bcdf2827468fa9c66e4b7ac1ba"
  "tests/roms/hm/M4_P128K_CR8K.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\tedb301630dae50a8470c1fb914f436434a150c886d9537d072275b751a9b4125")
