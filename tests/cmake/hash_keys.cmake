# The one list of keys tests/runner/hashes.txt must hold (D-16). Included by
# write_hashes.cmake (the writer checks what it generated), hash_inventory.cmake
# (the validator and its fixture), rom_coverage.cmake (the drift guard) and
# tests/CMakeLists.txt. It is self-contained so the CI hash-equality job, which
# runs hash_inventory.cmake from a checkout, needs nothing else.
#
# NESTURBATOR_HASH_GAMES   <key>:<file in tests/roms>. Each game is run for 180
#                          frames: boot frames 1, 30, 60, 120, 180 and two audio
#                          hashes under <key>/boot/.
# NESTURBATOR_ROM_HASH_EXEMPT  Top-level ROMs that need no hash rows (D-14).
#                          accuracycoin.nes is exempt: its NROM board is hashed
#                          through three games, and its scoreboard runs on every
#                          CI leg. Fuzz corpus seeds live outside tests/roms and
#                          run under fuzz.regress.
#
# Also hashed: the three DABG two-port movies (15 frame keys), nes-runner play
# frames 1, 30, 60, 120, 180, 300, 450 and 600 plus two audio keys, and each
# Holy Mapperel ROM: one frame key and two audio keys, again for each :save
# entry's .saved run.

cmake_minimum_required(VERSION 3.25)

include("${CMAKE_CURRENT_LIST_DIR}/../holymapperel/roms.cmake")

set(NESTURBATOR_HASH_GAMES
  "nesteroids:nesteroids.nes"
  "dabg:dabg.nes"
  "rhde:rhde.nes"
  "nesrunner:runner.nes")
set(NESTURBATOR_ROM_HASH_EXEMPT
  "accuracycoin.nes")

# Sets <out> to the sorted key list. A key is the text before " ticks " in a
# frame row, or the whole "... sha256" text of an audio row.
function(nesturbator_expected_hash_keys out)
  set(keys)
  foreach(game_entry IN LISTS NESTURBATOR_HASH_GAMES)
    string(REPLACE ":" ";" game_parts "${game_entry}")
    list(GET game_parts 0 game)
    foreach(frame IN ITEMS 1 30 60 120 180)
      list(APPEND keys "${game}/boot/frame ${frame}")
    endforeach()
    list(APPEND keys "${game}/boot/audio/pcm sha256")
    list(APPEND keys "${game}/boot/audio/transitions sha256")
  endforeach()
  foreach(port IN ITEMS p0 p1 both)
    foreach(frame IN ITEMS 1 30 60 120 180)
      list(APPEND keys "dabg/${port}/frame ${frame}")
    endforeach()
  endforeach()
  foreach(frame IN ITEMS 1 30 60 120 180 300 450 600)
    list(APPEND keys "nesrunner/play/frame ${frame}")
  endforeach()
  list(APPEND keys "nesrunner/play/audio/pcm sha256")
  list(APPEND keys "nesrunner/play/audio/transitions sha256")
  foreach(entry IN LISTS NESTURBATOR_HOLYMAPPEREL_ROMS)
    string(REPLACE ":" ";" parts "${entry}")
    list(GET parts 0 hm_key)
    list(GET parts 2 hm_n)
    set(variants "${hm_key}")
    if(entry MATCHES ":save(:|$)")
      list(APPEND variants "${hm_key}.saved")
    endif()
    foreach(variant IN LISTS variants)
      list(APPEND keys "holymapperel/${variant}/frame ${hm_n}")
      list(APPEND keys "holymapperel/${variant}/audio/pcm sha256")
      list(APPEND keys "holymapperel/${variant}/audio/transitions sha256")
    endforeach()
  endforeach()
  list(SORT keys)
  set(${out} "${keys}" PARENT_SCOPE)
endfunction()
