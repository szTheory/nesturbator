# The Holy Mapperel ROMs that tests/CMakeLists.txt, write_hashes.cmake and
# hash_inventory.cmake all read. Each entry is <key>:<file in tests/roms/hm>:<N>,
# where N is the frame whose result screen is read and hashed. Phases 8 and 9
# add one entry here and one line to the manifest list below per ROM.
set(NESTURBATOR_HOLYMAPPEREL_ROMS
  "m2:M2_P128K_CR8K_V.nes:100")

# The manifest.txt line of each ROM, so write_hashes.cmake can check provenance.
set(NESTURBATOR_HOLYMAPPEREL_MANIFEST
  "tests/roms/hm/M2_P128K_CR8K_V.nes\thttps://github.com/pinobatch/holy-mapperel/releases/download/v0.02/holy-mapperel-bin-0.02.7z\tc022622274ca8b83d214dea97e4388a6b0e92d8a\tZlib\tc7e83755bd9adbb7c705ea9f29535442af7390444632f9f824fcf4c00632069b")
