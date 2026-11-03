cmake_minimum_required(VERSION 3.25)

foreach(required IN ITEMS SOURCE_DIR MODE)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "provider_profile requires ${required}")
  endif()
endforeach()

get_filename_component(SOURCE_DIR "${SOURCE_DIR}" REALPATH)
if(NOT DEFINED PROFILE OR PROFILE STREQUAL "")
  set(PROFILE "${SOURCE_DIR}/docs/provider-ntsc.json")
endif()

function(provider_expect json field expected out_error)
  string(REPLACE "." ";" path "${field}")
  set(numeric_fields
    schema_version
    compatibility.abi compatibility.behaviour_revision compatibility.state_format
    region region.effective_value region.preference_value
    timing.fps_num timing.fps_den
    audio.sample_rate_num audio.sample_rate_den audio.initial_remainder
    audio.channels audio.minimum_capacity_samples audio.samples_per_tick_num
    audio.samples_per_tick_den audio.samples_per_ntsc_frame_num
    audio.samples_per_ntsc_frame_den audio.minimum_samples_per_ntsc_frame
    audio.maximum_samples_per_ntsc_frame
    clock.nominal_ticks_per_ntsc_frame input.logical_ports
    scenario.input_recipe.a_mask scenario.input_recipe.right_mask
    scenario.input_recipe.port_one_mask
    video.display_palette_entries video.width video.height video.minimum_pitch_pixels
    video.palette_index_mask video.emphasis_mask video.reserved_mask
    scenario.positive_frames scenario.refused_load_continuation_frames
    scenario.audio_window_first_frame scenario.audio_window_last_frame
    scenario.audio_window_samples
    rates.fps_num rates.fps_den
    rates.sample_rate_num rates.sample_rate_den completed_frames
    pcm.first_frame pcm.last_frame pcm.audio_count_min pcm.audio_count_max pcm.samples
    failure_mask integrity_checks.refused_load_status integrity_checks.continuation_frames)
  list(FIND numeric_fields "${field}" numeric_field_index)
  if(NOT numeric_field_index LESS 0 OR
     "${field}" MATCHES "^input\\.button_bits\\.[a-z]+$" OR
     "${field}" MATCHES "^header_version\\.(major|minor|patch|abi|behaviour_revision|state_format)$" OR
     "${field}" MATCHES "^library_version\\.(major|minor|patch|abi|behaviour_revision)$" OR
     "${field}" MATCHES "^input_recipe\\.(port0_a_mask|port0_right_mask|port1_mask)$")
    set(expected_type NUMBER)
  else()
    set(expected_type STRING)
  endif()
  string(JSON actual_type ERROR_VARIABLE type_error TYPE "${json}" ${path})
  if(type_error)
    set(${out_error} "${field}: ${type_error}" PARENT_SCOPE)
    return()
  elseif(NOT actual_type STREQUAL expected_type)
    set(${out_error} "${field}: expected JSON type ${expected_type}, found ${actual_type}" PARENT_SCOPE)
    return()
  endif()
  string(JSON actual ERROR_VARIABLE read_error GET "${json}" ${path})
  if(read_error)
    set(${out_error} "${field}: ${read_error}" PARENT_SCOPE)
  elseif(NOT "${actual}" STREQUAL "${expected}")
    set(${out_error} "${field}: expected ${expected}, found ${actual}" PARENT_SCOPE)
  else()
    set(${out_error} "" PARENT_SCOPE)
  endif()
endfunction()

function(provider_expect_boolean json field expected out_error)
  string(REPLACE "." ";" path "${field}")
  string(JSON actual_type ERROR_VARIABLE type_error TYPE "${json}" ${path})
  if(type_error)
    set(${out_error} "${field}: ${type_error}" PARENT_SCOPE)
    return()
  elseif(NOT actual_type STREQUAL "BOOLEAN")
    set(${out_error} "${field}: expected JSON type BOOLEAN, found ${actual_type}" PARENT_SCOPE)
    return()
  endif()
  string(JSON actual ERROR_VARIABLE read_error GET "${json}" ${path})
  if(read_error)
    set(${out_error} "${field}: ${read_error}" PARENT_SCOPE)
  elseif(actual AND expected STREQUAL "true")
    set(${out_error} "" PARENT_SCOPE)
  elseif(NOT actual AND expected STREQUAL "false")
    set(${out_error} "" PARENT_SCOPE)
  else()
    if(actual)
      set(actual_text true)
    else()
      set(actual_text false)
    endif()
    set(${out_error} "${field}: expected ${expected}, found ${actual_text}" PARENT_SCOPE)
  endif()
endfunction()

function(provider_get_define source name out_value out_error)
  string(REGEX MATCH "#define[ \t]+${name}[ \t]+([0-9]+)" macro_line "${source}")
  if(macro_line STREQUAL "")
    set(${out_error} "source macro ${name} is missing or not a decimal integer" PARENT_SCOPE)
    return()
  endif()
  set(${out_value} "${CMAKE_MATCH_1}" PARENT_SCOPE)
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(provider_gcd left right out_value)
  while(NOT right EQUAL 0)
    math(EXPR remainder "${left} % ${right}")
    set(left "${right}")
    set(right "${remainder}")
  endwhile()
  set(${out_value} "${left}" PARENT_SCOPE)
endfunction()

function(provider_check_recipe_array json field out_count out_error)
  string(REPLACE "." ";" path "${field}")
  string(JSON collection_type ERROR_VARIABLE type_error TYPE "${json}" ${path})
  if(type_error OR NOT collection_type STREQUAL "ARRAY")
    set(${out_error} "${field}: expected JSON type ARRAY, found ${collection_type}${type_error}" PARENT_SCOPE)
    return()
  endif()
  string(JSON item_count LENGTH "${json}" ${path})
  if(item_count GREATER 0)
    math(EXPR last_index "${item_count} - 1")
    foreach(index RANGE 0 ${last_index})
      string(JSON item_type ERROR_VARIABLE type_error TYPE "${json}" ${path} ${index})
      if(type_error OR NOT item_type STREQUAL "NUMBER")
        set(${out_error} "${field}[${index}]: expected JSON type NUMBER, found ${item_type}${type_error}" PARENT_SCOPE)
        return()
      endif()
    endforeach()
  endif()
  set(${out_count} "${item_count}" PARENT_SCOPE)
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(provider_profile_positive_predicates_valid profile out_error)
  set(field scenario.positive_predicates)
  string(JSON collection_type ERROR_VARIABLE type_error TYPE "${profile}" scenario positive_predicates)
  if(type_error)
    set(${out_error} "${field}: expected JSON type ARRAY, found ${collection_type}${type_error}" PARENT_SCOPE)
    return()
  elseif(NOT collection_type STREQUAL "ARRAY")
    set(${out_error} "${field}: expected JSON type ARRAY, found ${collection_type}" PARENT_SCOPE)
    return()
  endif()

  set(expected_predicates
    "source-mapped game state"
    "input-caused progress"
    "visible native pixel response"
    "non-silent gameplay PCM")
  string(JSON item_count LENGTH "${profile}" scenario positive_predicates)
  list(LENGTH expected_predicates expected_count)
  if(NOT item_count EQUAL expected_count)
    set(${out_error} "${field}: expected ${expected_count} consumer predicates, found ${item_count}" PARENT_SCOPE)
    return()
  endif()

  math(EXPR last_index "${expected_count} - 1")
  foreach(index RANGE 0 ${last_index})
    string(JSON item_type ERROR_VARIABLE item_type_error TYPE "${profile}" scenario positive_predicates ${index})
    if(item_type_error)
      set(${out_error} "${field}[${index}]: expected JSON type STRING, found ${item_type}${item_type_error}" PARENT_SCOPE)
      return()
    elseif(NOT item_type STREQUAL "STRING")
      set(${out_error} "${field}[${index}]: expected JSON type STRING, found ${item_type}" PARENT_SCOPE)
      return()
    endif()
    string(JSON actual GET "${profile}" scenario positive_predicates ${index})
    list(GET expected_predicates ${index} expected)
    if(NOT actual STREQUAL expected)
      set(${out_error} "${field}[${index}]: expected '${expected}', found '${actual}'" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(provider_profile_qualification_limits_valid profile out_error)
  foreach(pair IN ITEMS
      "no_content|test pattern and zero-valued silence, with NTSC timing"
      "refused_load|active cartridge and session remain unchanged"
      "jam|latched stop___SEMICOLON___ frame output fields are not written and native buffers are not results"
      "save_ram|core-owned span___SEMICOLON___ persistence and content identity are host-owned"
      "save_states|whole-machine snapshots___SEMICOLON___ host owns storage___SEMICOLON___ not qualified by this game scenario")
    string(REPLACE "|" ";" fields "${pair}")
    list(GET fields 0 field)
    list(GET fields 1 expected)
    string(REPLACE "___SEMICOLON___" ";" expected "${expected}")
    provider_expect("${profile}" "qualification_limits.${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  set(field qualification_limits.unqualified)
  string(JSON collection_type ERROR_VARIABLE type_error TYPE "${profile}" qualification_limits unqualified)
  if(type_error)
    set(${out_error} "${field}: expected JSON type ARRAY, found ${collection_type} (${type_error})" PARENT_SCOPE)
    return()
  elseif(NOT collection_type STREQUAL "ARRAY")
    set(${out_error} "${field}: expected JSON type ARRAY, found ${collection_type}" PARENT_SCOPE)
    return()
  endif()

  set(expected_unqualified
    "published package identity"
    "LibFuturo consumer integration or host playback"
    "PAL route"
    "physical device pacing"
    "browser or Playstead behavior"
    "battery restart"
    "machine snapshot interoperability"
    "hardware correctness"
    "general title or mapper compatibility")
  string(JSON item_count LENGTH "${profile}" qualification_limits unqualified)
  list(LENGTH expected_unqualified expected_count)
  if(NOT item_count EQUAL expected_count)
    set(${out_error} "${field}: expected ${expected_count} entries, found ${item_count}" PARENT_SCOPE)
    return()
  endif()

  math(EXPR last_index "${expected_count} - 1")
  foreach(index RANGE 0 ${last_index})
    string(JSON item_type ERROR_VARIABLE item_type_error TYPE "${profile}" qualification_limits unqualified ${index})
    if(item_type_error)
      set(${out_error} "${field}[${index}]: expected JSON type STRING, found ${item_type} (${item_type_error})" PARENT_SCOPE)
      return()
    elseif(NOT item_type STREQUAL "STRING")
      set(${out_error} "${field}[${index}]: expected JSON type STRING, found ${item_type}" PARENT_SCOPE)
      return()
    endif()
    string(JSON actual GET "${profile}" qualification_limits unqualified ${index})
    list(GET expected_unqualified ${index} expected)
    if(NOT actual STREQUAL expected)
      set(${out_error} "${field}[${index}]: expected '${expected}', found '${actual}'" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(provider_profile_recipe_arrays_valid profile out_error)
  # These compact arrays encode the frame ranges documented in
  # docs/nrom-gameplay.md. provider_validate also checks that guide's bound
  # digest, so contract mode pins both the prose and its executable recipe.
  foreach(recipe IN ITEMS
      "scenario.input_recipe.a_pulse_frames|3,57,3"
      "scenario.input_recipe.right_frames|80,110"
      "scenario.input_recipe.neutral_frames|58,79")
    string(REPLACE "|" ";" recipe_parts "${recipe}")
    list(GET recipe_parts 0 field)
    list(GET recipe_parts 1 expected_csv)
    string(REPLACE "," ";" expected_values "${expected_csv}")
    provider_check_recipe_array("${profile}" "${field}" item_count array_error)
    if(array_error)
      set(${out_error} "${array_error}" PARENT_SCOPE)
      return()
    endif()
    list(LENGTH expected_values expected_count)
    if(NOT item_count EQUAL expected_count)
      set(${out_error} "recipe mismatch: ${field} expected ${expected_count} entries, found ${item_count}" PARENT_SCOPE)
      return()
    endif()
    string(REPLACE "." ";" path "${field}")
    math(EXPR last_index "${expected_count} - 1")
    foreach(index RANGE 0 ${last_index})
      list(GET expected_values ${index} expected)
      string(JSON actual GET "${profile}" ${path} ${index})
      if(NOT actual EQUAL expected)
        set(${out_error} "recipe mismatch: ${field}[${index}] expected ${expected}, found ${actual}" PARENT_SCOPE)
        return()
      endif()
    endforeach()
  endforeach()
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(provider_recipe_arrays_match profile report out_error)
  foreach(pair IN ITEMS
      "a_pulse_frames;scenario.input_recipe.a_pulse_frames"
      "right_frames;scenario.input_recipe.right_frames"
      "neutral_frames;scenario.input_recipe.neutral_frames")
    list(GET pair 0 field)
    list(GET pair 1 profile_field)
    provider_check_recipe_array("${profile}" "${profile_field}" expected_count array_error)
    if(array_error)
      set(${out_error} "${array_error}" PARENT_SCOPE)
      return()
    endif()
    provider_check_recipe_array("${report}" "input_recipe.${field}" actual_count array_error)
    if(array_error)
      set(${out_error} "runtime.${array_error}" PARENT_SCOPE)
      return()
    endif()
    if(NOT actual_count EQUAL expected_count)
      set(${out_error} "runtime.input_recipe.${field}: array length differs" PARENT_SCOPE)
      return()
    endif()
    string(REPLACE "." ";" profile_path "${profile_field}")
    if(expected_count GREATER 0)
      math(EXPR last_index "${expected_count} - 1")
      foreach(index RANGE 0 ${last_index})
        string(JSON expected GET "${profile}" ${profile_path} ${index})
        string(JSON actual GET "${report}" input_recipe "${field}" ${index})
        if(NOT actual STREQUAL expected)
          set(${out_error} "runtime.input_recipe.${field}[${index}]: differs from the documented recipe" PARENT_SCOPE)
          return()
        endif()
      endforeach()
    endif()
  endforeach()
  set(${out_error} "" PARENT_SCOPE)
endfunction()

function(provider_array_as_numeric_key_object json out_json)
  cmake_parse_arguments(ARG "" "" "PATH" ${ARGN})
  string(JSON collection_type ERROR_VARIABLE type_error TYPE "${json}" ${ARG_PATH})
  if(type_error OR NOT collection_type STREQUAL "ARRAY")
    message(FATAL_ERROR "provider profile test helper requires an ARRAY, found ${collection_type}${type_error}")
  endif()
  string(JSON item_count LENGTH "${json}" ${ARG_PATH})
  set(object "{}")
  if(item_count GREATER 0)
    math(EXPR last_index "${item_count} - 1")
    foreach(index RANGE 0 ${last_index})
      string(JSON item GET "${json}" ${ARG_PATH} ${index})
      string(JSON object SET "${object}" "${index}" "${item}")
    endforeach()
  endif()
  string(JSON changed SET "${json}" ${ARG_PATH} "${object}")
  set(${out_json} "${changed}" PARENT_SCOPE)
endfunction()

function(provider_validate source_dir profile_path runtime_report out_error)
  set(${out_error} "" PARENT_SCOPE)
  if(NOT EXISTS "${profile_path}")
    set(${out_error} "profile.file: missing ${profile_path}" PARENT_SCOPE)
    return()
  endif()
  file(READ "${profile_path}" profile)
  provider_profile_recipe_arrays_valid("${profile}" recipe_error)
  if(recipe_error)
    set(${out_error} "${recipe_error}" PARENT_SCOPE)
    return()
  endif()
  provider_profile_positive_predicates_valid("${profile}" predicates_error)
  if(predicates_error)
    set(${out_error} "${predicates_error}" PARENT_SCOPE)
    return()
  endif()

  foreach(required IN ITEMS
      "${source_dir}/include/nesturbator.h"
      "${source_dir}/src/internal.h"
      "${source_dir}/src/frame.c"
      "${source_dir}/src/instance.c"
      "${source_dir}/docs/nrom-gameplay.md")
    if(NOT EXISTS "${required}")
      set(${out_error} "source.file: missing ${required}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  file(READ "${source_dir}/include/nesturbator.h" header)
  file(READ "${source_dir}/src/internal.h" internal)
  file(READ "${source_dir}/src/frame.c" frame)
  file(READ "${source_dir}/src/instance.c" instance)
  string(REPLACE "\n" " " header_flat "${header}")

  foreach(pair IN ITEMS
      "schema_version;1"
      "provider;Nesturbator"
      "qualification.status;candidate-only"
      "qualification.published_receipt;docs/provider-ntsc-release.json"
      "qualification.release_binding;pending")
    list(GET pair 0 field)
    list(GET pair 1 expected)
    provider_expect("${profile}" "${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  foreach(pair IN ITEMS
      "region.explicit_single_region_nes20_wins;true"
      "region.legacy_or_multiregion_uses_preference;true"
      "region.explicit_pal_is_qualified;false"
      "region.dendy_supported;false"
      "clock.ticks_are_cumulative;true"
      "clock.includes_soft_reset_cycles;true"
      "clock.instruction_and_dma_overshoot_is_preserved;true"
      "audio.remainder_is_carried_between_calls;true"
      "audio.consume_audio_count_only_after_ok;true")
    list(GET pair 0 field)
    list(GET pair 1 expected)
    provider_expect_boolean("${profile}" "${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  foreach(pair IN ITEMS
      "NESTURBATOR_ABI_VERSION;abi"
      "NESTURBATOR_BEHAVIOUR_REVISION;behaviour_revision"
      "NESTURBATOR_STATE_FORMAT_VERSION;state_format")
    list(GET pair 0 macro)
    list(GET pair 1 field)
    provider_get_define("${header}" "${macro}" expected macro_error)
    if(macro_error)
      set(${out_error} "${macro_error}" PARENT_SCOPE)
      return()
    endif()
    provider_expect("${profile}" "compatibility.${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "compatibility.${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
    set(expected_${field} "${expected}")
  endforeach()

  foreach(pair IN ITEMS
      "NESTURBATOR_BUTTON_A;a"
      "NESTURBATOR_BUTTON_B;b"
      "NESTURBATOR_BUTTON_SELECT;select"
      "NESTURBATOR_BUTTON_START;start"
      "NESTURBATOR_BUTTON_UP;up"
      "NESTURBATOR_BUTTON_DOWN;down"
      "NESTURBATOR_BUTTON_LEFT;left"
      "NESTURBATOR_BUTTON_RIGHT;right")
    list(GET pair 0 name)
    list(GET pair 1 field)
    string(REGEX MATCH "${name}[ \t]*=[ \t]*1u[ \t]*<<[ \t]*([0-9]+)" button_line "${header}")
    if(button_line STREQUAL "")
      set(${out_error} "input.${field}: public button bit is missing" PARENT_SCOPE)
      return()
    endif()
    math(EXPR expected "1 << ${CMAKE_MATCH_1}")
    provider_expect("${profile}" "input.button_bits.${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "input.button_bits.${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  foreach(pair IN ITEMS
      "input.port_zero_address;0x4016"
      "input.port_one_address;0x4017"
      "input.physical_device_mapping;host-owned"
      "input.sampling;latest masks sampled at the start of each successful frame call")
    list(GET pair 0 field)
    list(GET pair 1 expected)
    provider_expect("${profile}" "${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  foreach(pair IN ITEMS
      "NESTURBATOR_VERSION_MAJOR;version_major"
      "NESTURBATOR_VERSION_MINOR;version_minor"
      "NESTURBATOR_VERSION_PATCH;version_patch")
    list(GET pair 0 macro)
    list(GET pair 1 field)
    provider_get_define("${header}" "${macro}" expected macro_error)
    if(macro_error)
      set(${out_error} "${macro_error}" PARENT_SCOPE)
      return()
    endif()
    set(${field} "${expected}")
  endforeach()

  if(NOT header MATCHES "(NESTURBATOR_REGION_NTSC)[ \t]*=[ \t]*([0-9]+)")
    set(${out_error} "region.source: effective NTSC enum value is missing" PARENT_SCOPE)
    return()
  endif()
  set(region_name "${CMAKE_MATCH_1}")
  set(region_value "${CMAKE_MATCH_2}")
  if(NOT header MATCHES "(NESTURBATOR_REGION_PREFERENCE_NTSC)[ \t]*=[ \t]*([0-9]+)")
    set(${out_error} "region.source: NTSC preference enum value is missing" PARENT_SCOPE)
    return()
  endif()
  set(preference_name "${CMAKE_MATCH_1}")
  set(preference_value "${CMAKE_MATCH_2}")
  provider_expect("${profile}" "region.preference_name" "${preference_name}" mismatch)
  if(mismatch)
    set(${out_error} "${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "region.effective_name" "${region_name}" mismatch)
  if(mismatch)
    set(${out_error} "${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "region.effective_value" "${region_value}" mismatch)
  if(mismatch)
    set(${out_error} "region.effective_value: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "region.preference_value" "${preference_value}" mismatch)
  if(mismatch)
    set(${out_error} "region.preference_value: ${mismatch}" PARENT_SCOPE)
    return()
  endif()

  if(NOT header_flat MATCHES "fps_num;[^;]*NTSC: *([0-9]+) */ *([0-9]+)" OR
     NOT header_flat MATCHES "sample_rate_num;[^;]*samples per second: *([0-9]+) */ *([0-9]+)")
    set(${out_error} "rates.header: exact NTSC frame and sample-rate fractions are missing" PARENT_SCOPE)
    return()
  endif()
  set(fps_num "${CMAKE_MATCH_1}")
  # Re-run each match separately because CMAKE_MATCH_n is overwritten by the later expression.
  if(NOT header_flat MATCHES "fps_num;[^;]*NTSC: *([0-9]+) */ *([0-9]+)")
    set(${out_error} "rates.header: NTSC frame fraction is missing" PARENT_SCOPE)
    return()
  endif()
  set(fps_num "${CMAKE_MATCH_1}")
  set(fps_den "${CMAKE_MATCH_2}")
  if(NOT header_flat MATCHES "sample_rate_num;[^;]*samples per second: *([0-9]+) */ *([0-9]+)")
    set(${out_error} "rates.header: sample-rate fraction is missing" PARENT_SCOPE)
    return()
  endif()
  set(rate_num "${CMAKE_MATCH_1}")
  set(rate_den "${CMAKE_MATCH_2}")
  provider_expect("${profile}" "timing.fps_num" "${fps_num}" mismatch)
  if(mismatch)
    set(${out_error} "timing.fps_num: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "timing.fps_den" "${fps_den}" mismatch)
  if(mismatch)
    set(${out_error} "timing.fps_den: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "audio.sample_rate_num" "${rate_num}" mismatch)
  if(mismatch)
    set(${out_error} "audio.sample_rate_num: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "audio.sample_rate_den" "${rate_den}" mismatch)
  if(mismatch)
    set(${out_error} "audio.sample_rate_den: ${mismatch}" PARENT_SCOPE)
    return()
  endif()

  provider_get_define("${internal}" "NESTURBATOR_TICKS_PER_FRAME" frame_ticks macro_error)
  if(macro_error)
    set(${out_error} "${macro_error}" PARENT_SCOPE)
    return()
  endif()
  provider_get_define("${internal}" "NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD" sample_num macro_error)
  if(macro_error)
    set(${out_error} "${macro_error}" PARENT_SCOPE)
    return()
  endif()
  provider_get_define("${internal}" "NESTURBATOR_AUDIO_TICKS_PER_PERIOD" sample_den macro_error)
  if(macro_error)
    set(${out_error} "${macro_error}" PARENT_SCOPE)
    return()
  endif()
  math(EXPR frame_samples_num "${frame_ticks} * ${sample_num}")
  set(frame_samples_den "${sample_den}")
  provider_gcd("${frame_samples_num}" "${frame_samples_den}" divisor)
  math(EXPR frame_samples_num "${frame_samples_num} / ${divisor}")
  math(EXPR frame_samples_den "${frame_samples_den} / ${divisor}")
  math(EXPR samples_min "${frame_ticks} * ${sample_num} / ${sample_den}")
  math(EXPR samples_max "(${frame_ticks} * ${sample_num} + ${sample_den} - 1) / ${sample_den}")
  foreach(pair IN ITEMS
      "clock.nominal_ticks_per_ntsc_frame;${frame_ticks}"
      "audio.samples_per_tick_num;${sample_num}"
      "audio.samples_per_tick_den;${sample_den}"
      "audio.samples_per_ntsc_frame_num;${frame_samples_num}"
      "audio.samples_per_ntsc_frame_den;${frame_samples_den}"
      "audio.minimum_samples_per_ntsc_frame;${samples_min}"
      "audio.maximum_samples_per_ntsc_frame;${samples_max}")
    list(GET pair 0 field)
    list(GET pair 1 expected)
    provider_expect("${profile}" "${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  foreach(pair IN ITEMS
      "audio.channels;1"
      "audio.minimum_capacity_samples;${samples_max}"
      "audio.format;signed int16 PCM"
      "video.format;caller-owned uint16_t native pixels"
      "video.display_palette_entries;512"
      "video.display_palette_format;XRGB8888"
      "clock.tick_unit;half-master-clock periods"
      "input.logical_ports;2"
      "input.button_bits.a;1"
      "input.button_bits.b;2"
      "input.button_bits.select;4"
      "input.button_bits.start;8"
      "input.button_bits.up;16"
      "input.button_bits.down;32"
      "input.button_bits.left;64"
      "input.button_bits.right;128"
      "scenario.positive_frames;110"
      "scenario.refused_load_continuation_frames;30"
      "scenario.audio_window_first_frame;61"
      "scenario.audio_window_last_frame;76"
      "scenario.audio_window_samples;12779")
    list(GET pair 0 field)
    list(GET pair 1 expected)
    provider_expect("${profile}" "${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  provider_expect_boolean("${profile}" "scenario.input_recipe.one_based_frames" "true" mismatch)
  if(mismatch)
    set(${out_error} "scenario.input_recipe.one_based_frames: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  foreach(pair IN ITEMS "a_mask;a" "right_mask;right")
    list(GET pair 0 recipe_field)
    list(GET pair 1 button_field)
    string(JSON button_mask GET "${profile}" input button_bits "${button_field}")
    provider_expect("${profile}" "scenario.input_recipe.${recipe_field}" "${button_mask}" mismatch)
    if(mismatch)
      set(${out_error} "${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  provider_expect("${profile}" "scenario.input_recipe.port_one_mask" "0" mismatch)
  if(mismatch)
    set(${out_error} "${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_profile_qualification_limits_valid("${profile}" qualification_error)
  if(qualification_error)
    set(${out_error} "${qualification_error}" PARENT_SCOPE)
    return()
  endif()

  file(STRINGS "${source_dir}/tests/roms/manifest.txt" dabg_rows
    REGEX "^tests/roms/dabg\\.nes[ \t]+")
  list(LENGTH dabg_rows dabg_row_count)
  if(NOT dabg_row_count EQUAL 1)
    set(${out_error} "scenario.manifest: expected one DABG ROM manifest row" PARENT_SCOPE)
    return()
  endif()
  list(GET dabg_rows 0 dabg_row)
  string(REPLACE "\t" ";" dabg_fields "${dabg_row}")
  list(LENGTH dabg_fields dabg_field_count)
  if(NOT dabg_field_count EQUAL 5)
    set(${out_error} "scenario.manifest: DABG row is malformed" PARENT_SCOPE)
    return()
  endif()
  list(GET dabg_fields 0 dabg_path)
  list(GET dabg_fields 1 dabg_source)
  list(GET dabg_fields 2 dabg_revision)
  list(GET dabg_fields 3 dabg_license)
  list(GET dabg_fields 4 dabg_digest)
  string(LENGTH "${dabg_digest}" dabg_digest_length)
  if(NOT dabg_path STREQUAL "tests/roms/dabg.nes" OR
     NOT dabg_source MATCHES "NovaSquirrel/DABG" OR
     NOT dabg_revision STREQUAL "5ecc60b6af3f726851bfeb1c2555388c5953e0c0" OR
     NOT dabg_license STREQUAL "Zlib" OR dabg_digest_length LESS 64 OR
     dabg_digest_length GREATER 64 OR NOT dabg_digest MATCHES "^[0-9a-f]+$")
    set(${out_error} "scenario.manifest: DABG source, pin, license, or digest differs" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "scenario.rom_sha256" "${dabg_digest}" mismatch)
  if(mismatch)
    set(${out_error} "scenario.rom_sha256: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  if(EXISTS "${source_dir}/tests/roms/dabg.nes")
    file(SHA256 "${source_dir}/tests/roms/dabg.nes" local_rom_digest)
    if(NOT local_rom_digest STREQUAL dabg_digest)
      set(${out_error} "scenario.rom_sha256: permitted fixture bytes differ from the manifest" PARENT_SCOPE)
      return()
    endif()
  endif()

  string(FIND "${frame}" "(uint64_t)inst->audio_rem + (uint64_t)frame_ticks * sample_num" accumulator_index)
  string(FIND "${frame}" "acc / sample_den" division_index)
  string(FIND "${frame}" "acc % sample_den" remainder_index)
  string(FIND "${instance}" "audio_rem = 0" initial_phase_index)
  if(accumulator_index LESS 0 OR division_index LESS 0 OR remainder_index LESS 0 OR
     initial_phase_index LESS 0)
    set(${out_error} "audio.arithmetic: source no longer uses a zero-phase integer remainder accumulator" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "audio.initial_remainder" "0" mismatch)
  if(mismatch)
    set(${out_error} "audio.initial_remainder: ${mismatch}" PARENT_SCOPE)
    return()
  endif()

  if(NOT header_flat MATCHES "width;[^;]*256" OR NOT header_flat MATCHES "height;[^;]*240" OR
     NOT header_flat MATCHES "video_pitch;[^;]*at least 256")
    set(${out_error} "video.header: dimensions or minimum pitch differ from the documented contract" PARENT_SCOPE)
    return()
  endif()
  foreach(pair IN ITEMS "video.width;256" "video.height;240" "video.minimum_pitch_pixels;256"
                        "video.palette_index_mask;63" "video.emphasis_mask;448" "video.reserved_mask;65024")
    list(GET pair 0 field)
    list(GET pair 1 expected)
    provider_expect("${profile}" "${field}" "${expected}" mismatch)
    if(mismatch)
      set(${out_error} "${field}: ${mismatch}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  provider_expect("${profile}" "scenario.game" "DABG" mismatch)
  if(mismatch)
    set(${out_error} "scenario.game: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "scenario.guide" "docs/nrom-gameplay.md" mismatch)
  if(mismatch)
    set(${out_error} "scenario.guide: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  provider_expect("${profile}" "scenario.rom" "tests/roms/dabg.nes" mismatch)
  if(mismatch)
    set(${out_error} "scenario.rom: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  if(NOT EXISTS "${source_dir}/docs/nrom-gameplay.md")
    set(${out_error} "scenario.guide: source-backed guide is missing" PARENT_SCOPE)
    return()
  endif()
  file(SHA256 "${source_dir}/docs/nrom-gameplay.md" scenario_digest)
  provider_expect("${profile}" "scenario.guide_sha256" "${scenario_digest}" mismatch)
  if(mismatch)
    set(${out_error} "scenario.guide_sha256: ${mismatch}" PARENT_SCOPE)
    return()
  endif()
  if(NOT EXISTS "${source_dir}/tests/roms/manifest.txt")
    set(${out_error} "scenario.manifest: permitted ROM manifest is missing" PARENT_SCOPE)
    return()
  endif()
  if(DEFINED runtime_report AND NOT runtime_report STREQUAL "")
    if(NOT EXISTS "${runtime_report}")
      set(${out_error} "runtime.report: missing ${runtime_report}" PARENT_SCOPE)
      return()
    endif()
    file(READ "${runtime_report}" report)
    foreach(pair IN ITEMS
        "game;DABG" "candidate_status;unpublished" "scenario;positive"
        "region;${region_value}" "video.width;256" "video.height;240"
        "rates.fps_num;${fps_num}" "rates.fps_den;${fps_den}"
        "rates.sample_rate_num;${rate_num}" "rates.sample_rate_den;${rate_den}"
        "completed_frames;110" "pcm.first_frame;61" "pcm.last_frame;76"
        "pcm.audio_count_min;${samples_min}" "pcm.audio_count_max;${samples_max}"
        "pcm.samples;12779" "failure_mask;0"
        "integrity_checks.refused_load_status;7" "integrity_checks.continuation_frames;30")
      list(GET pair 0 field)
      list(GET pair 1 expected)
      provider_expect("${report}" "${field}" "${expected}" mismatch)
      if(mismatch)
        set(${out_error} "runtime.${field}: ${mismatch}" PARENT_SCOPE)
        return()
      endif()
    endforeach()
    foreach(field IN ITEMS "predicates.game_state" "predicates.input_caused_progress"
                          "predicates.visible_native_response" "predicates.non_silent_gameplay_pcm"
                          "integrity_checks.executed" "integrity_checks.refusal_preserved"
                          "integrity_checks.continuation_equal" "integrity_checks.buffer_edges_passed"
                          "integrity_checks.all_positive_checks_passed" "passed")
      provider_expect_boolean("${report}" "${field}" "true" mismatch)
      if(mismatch)
        set(${out_error} "runtime.${field}: ${mismatch}" PARENT_SCOPE)
        return()
      endif()
    endforeach()
    foreach(pair IN ITEMS
        "header_version.major;${version_major}" "header_version.minor;${version_minor}"
        "header_version.patch;${version_patch}" "header_version.abi;${expected_abi}"
        "header_version.behaviour_revision;${expected_behaviour_revision}"
        "header_version.state_format;${expected_state_format}"
        "library_version.major;${version_major}" "library_version.minor;${version_minor}"
        "library_version.patch;${version_patch}" "library_version.abi;${expected_abi}"
        "library_version.behaviour_revision;${expected_behaviour_revision}")
      list(GET pair 0 field)
      list(GET pair 1 expected)
      provider_expect("${report}" "${field}" "${expected}" mismatch)
      if(mismatch)
        set(${out_error} "runtime.${field}: ${mismatch}" PARENT_SCOPE)
        return()
      endif()
    endforeach()

    foreach(pair IN ITEMS
        "port0_a_mask;scenario.input_recipe.a_mask"
        "port0_right_mask;scenario.input_recipe.right_mask"
        "port1_mask;scenario.input_recipe.port_one_mask")
      list(GET pair 0 field)
      list(GET pair 1 profile_field)
      string(REPLACE "." ";" profile_path "${profile_field}")
      string(JSON expected GET "${profile}" ${profile_path})
      provider_expect("${report}" "input_recipe.${field}" "${expected}" mismatch)
      if(mismatch)
        set(${out_error} "runtime.input_recipe.${field}: ${mismatch}" PARENT_SCOPE)
        return()
      endif()
    endforeach()
    provider_recipe_arrays_match("${profile}" "${report}" recipe_error)
    if(recipe_error)
      set(${out_error} "${recipe_error}" PARENT_SCOPE)
      return()
    endif()
  endif()
endfunction()

function(provider_profile_selftest source_dir profile_path work)
  if(NOT EXISTS "${profile_path}")
    message(FATAL_ERROR "provider profile self-test requires ${profile_path}")
  endif()
  file(READ "${profile_path}" profile)
  provider_validate("${source_dir}" "${profile_path}" "" positive_error)
  if(positive_error)
    message(FATAL_ERROR "provider profile self-test positive fixture failed: ${positive_error}")
  endif()

  string(JSON report SET "{}" input_recipe "{}")
  string(JSON recipe GET "${profile}" scenario input_recipe)
  string(JSON report SET "${report}" input_recipe "${recipe}")
  provider_recipe_arrays_match("${profile}" "${report}" recipe_error)
  if(recipe_error)
    message(FATAL_ERROR "provider profile self-test recipe fixture failed: ${recipe_error}")
  endif()
  foreach(pair IN ITEMS
      "a_pulse_frames|scenario.input_recipe.a_pulse_frames|input_recipe.a_pulse_frames"
      "right_frames|scenario.input_recipe.right_frames|input_recipe.right_frames"
      "neutral_frames|scenario.input_recipe.neutral_frames|input_recipe.neutral_frames")
    string(REPLACE "|" ";" fields "${pair}")
    list(GET fields 0 label)
    list(GET fields 1 profile_path_text)
    list(GET fields 2 report_path_text)
    string(REPLACE "." ";" profile_path "${profile_path_text}")
    string(REPLACE "." ";" report_path "${report_path_text}")
    provider_array_as_numeric_key_object("${profile}" profile_mutated PATH ${profile_path})
    provider_array_as_numeric_key_object("${report}" report_mutated PATH ${report_path})
    provider_recipe_arrays_match("${profile_mutated}" "${report}" profile_array_error)
    if(NOT profile_array_error MATCHES "expected JSON type ARRAY.*OBJECT")
      message(FATAL_ERROR "provider profile ${label} object control reached '${profile_array_error}', expected an ARRAY type rejection")
    endif()
    provider_recipe_arrays_match("${profile}" "${report_mutated}" report_array_error)
    if(NOT report_array_error MATCHES "expected JSON type ARRAY.*OBJECT")
      message(FATAL_ERROR "provider runtime ${label} object control reached '${report_array_error}', expected an ARRAY type rejection")
    endif()
  endforeach()
  string(JSON profile_bad_element SET "${profile}" scenario input_recipe a_pulse_frames 0 "\"3\"")
  provider_recipe_arrays_match("${profile_bad_element}" "${report}" profile_element_error)
  if(NOT profile_element_error MATCHES "expected JSON type NUMBER.*STRING")
    message(FATAL_ERROR "provider profile recipe element control reached '${profile_element_error}', expected a NUMBER type rejection")
  endif()
  string(JSON report_bad_element SET "${report}" input_recipe a_pulse_frames 0 "\"3\"")
  provider_recipe_arrays_match("${profile}" "${report_bad_element}" report_element_error)
  if(NOT report_element_error MATCHES "expected JSON type NUMBER.*STRING")
    message(FATAL_ERROR "provider runtime recipe element control reached '${report_element_error}', expected a NUMBER type rejection")
  endif()

  string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef run_id)
  set(control_work "${work}/profile-controls-${run_id}")
  if(EXISTS "${control_work}" OR IS_SYMLINK "${control_work}")
    message(FATAL_ERROR "provider profile self-test scratch already exists: ${control_work}")
  endif()
  file(MAKE_DIRECTORY "${control_work}")
  set(control_marker "${control_work}/.provider-profile-controls-owned")
  file(WRITE "${control_marker}" "Nesturbator provider profile controls v1\n")
  string(JSON mutated_recipe SET "${profile}" scenario input_recipe a_pulse_frames 0 4)
  set(mutated_recipe_path "${control_work}/mutated-numeric-recipe.json")
  file(WRITE "${mutated_recipe_path}" "${mutated_recipe}\n")
  provider_validate("${source_dir}" "${mutated_recipe_path}" "" mutated_recipe_error)
  if(NOT mutated_recipe_error MATCHES "^recipe mismatch: scenario\\.input_recipe\\.a_pulse_frames\\[0\\]")
    message(FATAL_ERROR
      "provider numeric recipe control reached '${mutated_recipe_error}', expected a recipe mismatch")
  endif()

  foreach(pair IN ITEMS "a_mask;a" "right_mask;right")
    list(GET pair 0 recipe_field)
    list(GET pair 1 button_field)
    string(JSON expected_mask GET "${profile}" input button_bits "${button_field}")
    math(EXPR mutated_mask "${expected_mask} + 1")
    string(JSON mutated_mask_profile SET "${profile}" scenario input_recipe "${recipe_field}" "${mutated_mask}")
    set(mutated_mask_path "${control_work}/${recipe_field}-mask.json")
    file(WRITE "${mutated_mask_path}" "${mutated_mask_profile}\n")
    provider_validate("${source_dir}" "${mutated_mask_path}" "" mutated_mask_error)
    set(expected_mask_error
      "scenario.input_recipe.${recipe_field}: expected ${expected_mask}, found ${mutated_mask}")
    if(NOT mutated_mask_error STREQUAL expected_mask_error)
      message(FATAL_ERROR
        "provider scenario ${recipe_field} mask control reached '${mutated_mask_error}', expected '${expected_mask_error}'")
    endif()
  endforeach()

  string(JSON nonzero_port_one SET "${profile}" scenario input_recipe port_one_mask 1)
  set(nonzero_port_one_path "${control_work}/port-one-mask-nonzero.json")
  file(WRITE "${nonzero_port_one_path}" "${nonzero_port_one}\n")
  provider_validate("${source_dir}" "${nonzero_port_one_path}" "" nonzero_port_one_error)
  if(NOT nonzero_port_one_error STREQUAL
      "scenario.input_recipe.port_one_mask: expected 0, found 1")
    message(FATAL_ERROR
      "provider port-one mask value control reached '${nonzero_port_one_error}', expected a zero-mask rejection")
  endif()

  string(JSON string_port_one SET "${profile}" scenario input_recipe port_one_mask "\"0\"")
  set(string_port_one_path "${control_work}/port-one-mask-string.json")
  file(WRITE "${string_port_one_path}" "${string_port_one}\n")
  provider_validate("${source_dir}" "${string_port_one_path}" "" string_port_one_error)
  if(NOT string_port_one_error STREQUAL
      "scenario.input_recipe.port_one_mask: expected JSON type NUMBER, found STRING")
    message(FATAL_ERROR
      "provider port-one mask type control reached '${string_port_one_error}', expected a NUMBER type rejection")
  endif()

  set(object_predicates "{}")
  string(JSON object_predicates SET "${object_predicates}" "0" "\"source-mapped game state\"")
  string(JSON object_predicates SET "${object_predicates}" "1" "\"input-caused progress\"")
  string(JSON object_predicates SET "${object_predicates}" "2" "\"visible native pixel response\"")
  string(JSON object_predicates SET "${object_predicates}" "3" "\"non-silent gameplay PCM\"")
  string(JSON object_mutated_profile SET "${profile}" scenario positive_predicates "${object_predicates}")
  set(object_predicates_path "${control_work}/positive-predicates-object.json")
  file(WRITE "${object_predicates_path}" "${object_mutated_profile}\n")
  provider_validate("${source_dir}" "${object_predicates_path}" "" object_predicates_error)
  if(NOT object_predicates_error MATCHES
      "^scenario\\.positive_predicates: expected JSON type ARRAY, found OBJECT")
    message(FATAL_ERROR
      "provider positive-predicates collection control reached '${object_predicates_error}', expected a named ARRAY rejection")
  endif()

  string(JSON wrong_type_predicate SET "${profile}" scenario positive_predicates 0 7)
  set(wrong_type_predicate_path "${control_work}/positive-predicates-wrong-type.json")
  file(WRITE "${wrong_type_predicate_path}" "${wrong_type_predicate}\n")
  provider_validate("${source_dir}" "${wrong_type_predicate_path}" "" wrong_type_predicate_error)
  if(NOT wrong_type_predicate_error STREQUAL
      "scenario.positive_predicates[0]: expected JSON type STRING, found NUMBER")
    message(FATAL_ERROR
      "provider positive-predicates element type control reached '${wrong_type_predicate_error}', expected a STRING type rejection")
  endif()

  string(JSON changed_predicate SET "${profile}" scenario positive_predicates 0 "\"unmapped state\"")
  set(changed_predicate_path "${control_work}/positive-predicates-changed-entry.json")
  file(WRITE "${changed_predicate_path}" "${changed_predicate}\n")
  provider_validate("${source_dir}" "${changed_predicate_path}" "" changed_predicate_error)
  if(NOT changed_predicate_error STREQUAL
      "scenario.positive_predicates[0]: expected 'source-mapped game state', found 'unmapped state'")
    message(FATAL_ERROR
      "provider positive-predicates array mutation reached '${changed_predicate_error}', expected a named exact-entry rejection")
  endif()

  foreach(pair IN ITEMS
      "preference_name;NESTURBATOR_REGION_PREFERENCE_NTSC"
      "effective_name;NESTURBATOR_REGION_NTSC")
    list(GET pair 0 field)
    list(GET pair 1 expected_name)
    string(JSON wrong_name SET "${profile}" region "${field}" "\"unrelated-region-name\"")
    set(wrong_name_path "${control_work}/region-${field}-wrong-name.json")
    file(WRITE "${wrong_name_path}" "${wrong_name}\n")
    provider_validate("${source_dir}" "${wrong_name_path}" "" wrong_name_error)
    set(expected_name_error "region.${field}: expected ${expected_name}, found unrelated-region-name")
    if(NOT wrong_name_error STREQUAL expected_name_error)
      message(FATAL_ERROR
        "provider region ${field} wrong-name control reached '${wrong_name_error}', expected '${expected_name_error}'")
    endif()

    string(JSON wrong_type SET "${profile}" region "${field}" 42)
    set(wrong_type_path "${control_work}/region-${field}-wrong-type.json")
    file(WRITE "${wrong_type_path}" "${wrong_type}\n")
    provider_validate("${source_dir}" "${wrong_type_path}" "" wrong_type_error)
    set(expected_type_error "region.${field}: expected JSON type STRING, found NUMBER")
    if(NOT wrong_type_error STREQUAL expected_type_error)
      message(FATAL_ERROR
        "provider region ${field} wrong-type control reached '${wrong_type_error}', expected '${expected_type_error}'")
    endif()
  endforeach()

  foreach(field IN ITEMS no_content refused_load jam save_ram save_states)
    string(JSON changed_claim SET "${profile}" qualification_limits "${field}" "\"changed qualification claim\"")
    set(changed_claim_path "${control_work}/qualification-${field}-changed.json")
    file(WRITE "${changed_claim_path}" "${changed_claim}\n")
    provider_validate("${source_dir}" "${changed_claim_path}" "" changed_claim_error)
    if(NOT changed_claim_error MATCHES "^qualification_limits\\.${field}: expected .*, found changed qualification claim$")
      message(FATAL_ERROR
        "provider qualification_limits.${field} changed-value control reached '${changed_claim_error}', expected a named exact-value rejection")
    endif()

    string(JSON missing_claim REMOVE "${profile}" qualification_limits "${field}")
    set(missing_claim_path "${control_work}/qualification-${field}-missing.json")
    file(WRITE "${missing_claim_path}" "${missing_claim}\n")
    provider_validate("${source_dir}" "${missing_claim_path}" "" missing_claim_error)
    if(NOT missing_claim_error MATCHES "^qualification_limits\\.${field}:")
      message(FATAL_ERROR
        "provider qualification_limits.${field} missing-field control reached '${missing_claim_error}', expected a named missing-field rejection")
    endif()

    string(JSON wrong_type_claim SET "${profile}" qualification_limits "${field}" 42)
    set(wrong_type_claim_path "${control_work}/qualification-${field}-wrong-type.json")
    file(WRITE "${wrong_type_claim_path}" "${wrong_type_claim}\n")
    provider_validate("${source_dir}" "${wrong_type_claim_path}" "" wrong_type_claim_error)
    if(NOT wrong_type_claim_error STREQUAL
        "qualification_limits.${field}: expected JSON type STRING, found NUMBER")
      message(FATAL_ERROR
        "provider qualification_limits.${field} wrong-type control reached '${wrong_type_claim_error}', expected a STRING type rejection")
    endif()
  endforeach()

  string(JSON changed_unqualified SET "${profile}" qualification_limits unqualified 0 "\"changed exclusion\"")
  set(changed_unqualified_path "${control_work}/qualification-unqualified-changed.json")
  file(WRITE "${changed_unqualified_path}" "${changed_unqualified}\n")
  provider_validate("${source_dir}" "${changed_unqualified_path}" "" changed_unqualified_error)
  if(NOT changed_unqualified_error STREQUAL
      "qualification_limits.unqualified[0]: expected 'published package identity', found 'changed exclusion'")
    message(FATAL_ERROR
      "provider unqualified changed-member control reached '${changed_unqualified_error}', expected an exact-member rejection")
  endif()

  string(JSON missing_unqualified_item REMOVE "${profile}" qualification_limits unqualified 0)
  set(missing_unqualified_item_path "${control_work}/qualification-unqualified-missing-item.json")
  file(WRITE "${missing_unqualified_item_path}" "${missing_unqualified_item}\n")
  provider_validate("${source_dir}" "${missing_unqualified_item_path}" "" missing_unqualified_item_error)
  if(NOT missing_unqualified_item_error STREQUAL
      "qualification_limits.unqualified: expected 9 entries, found 8")
    message(FATAL_ERROR
      "provider unqualified missing-member control reached '${missing_unqualified_item_error}', expected an exact-length rejection")
  endif()

  string(JSON wrong_type_unqualified_item SET "${profile}" qualification_limits unqualified 0 7)
  set(wrong_type_unqualified_item_path "${control_work}/qualification-unqualified-wrong-item-type.json")
  file(WRITE "${wrong_type_unqualified_item_path}" "${wrong_type_unqualified_item}\n")
  provider_validate("${source_dir}" "${wrong_type_unqualified_item_path}" "" wrong_type_unqualified_item_error)
  if(NOT wrong_type_unqualified_item_error STREQUAL
      "qualification_limits.unqualified[0]: expected JSON type STRING, found NUMBER")
    message(FATAL_ERROR
      "provider unqualified wrong-member-type control reached '${wrong_type_unqualified_item_error}', expected a STRING type rejection")
  endif()

  string(JSON unqualified_object SET "{}" item "\"placeholder\"")
  string(JSON wrong_type_unqualified_array SET "${profile}" qualification_limits unqualified "${unqualified_object}")
  set(wrong_type_unqualified_array_path "${control_work}/qualification-unqualified-wrong-array-type.json")
  file(WRITE "${wrong_type_unqualified_array_path}" "${wrong_type_unqualified_array}\n")
  provider_validate("${source_dir}" "${wrong_type_unqualified_array_path}" "" wrong_type_unqualified_array_error)
  if(NOT wrong_type_unqualified_array_error MATCHES
      "^qualification_limits\\.unqualified: expected JSON type ARRAY, found OBJECT")
    message(FATAL_ERROR
      "provider unqualified wrong-array-type control reached '${wrong_type_unqualified_array_error}', expected an ARRAY type rejection")
  endif()

  string(JSON missing_unqualified_array REMOVE "${profile}" qualification_limits unqualified)
  set(missing_unqualified_array_path "${control_work}/qualification-unqualified-missing-array.json")
  file(WRITE "${missing_unqualified_array_path}" "${missing_unqualified_array}\n")
  provider_validate("${source_dir}" "${missing_unqualified_array_path}" "" missing_unqualified_array_error)
  if(NOT missing_unqualified_array_error MATCHES "^qualification_limits\\.unqualified: expected JSON type ARRAY")
    message(FATAL_ERROR
      "provider unqualified missing-array control reached '${missing_unqualified_array_error}', expected a missing-array rejection")
  endif()

  set(cases
    "timing.fps_num|1|timing.fps_num"
    "audio.maximum_samples_per_ntsc_frame|800|audio.maximum_samples_per_ntsc_frame"
    "video.reserved_mask|1|video.reserved_mask"
    "scenario.guide_sha256|\"0000000000000000000000000000000000000000000000000000000000000000\"|scenario.guide_sha256"
    "compatibility.abi|99|compatibility.abi"
    "region.effective_value|1|region.effective_value"
    "compatibility.abi|\"1\"|compatibility.abi"
    "provider|1|provider"
    "region.explicit_single_region_nes20_wins|\"true\"|region.explicit_single_region_nes20_wins")
  foreach(case IN LISTS cases)
    string(REPLACE "|" ";" parts "${case}")
    list(GET parts 0 field)
    list(GET parts 1 replacement)
    list(GET parts 2 expected_reason)
    string(REPLACE "." ";" path "${field}")
    string(JSON mutated SET "${profile}" ${path} ${replacement})
    set(mutated_path "${control_work}/${field}.json")
    file(WRITE "${mutated_path}" "${mutated}\n")
    provider_validate("${source_dir}" "${mutated_path}" "" actual_error)
    if(NOT actual_error MATCHES "^${expected_reason}:")
      message(FATAL_ERROR
        "provider profile control ${field} failed for the wrong reason: ${actual_error}")
    endif()
  endforeach()

  if(IS_SYMLINK "${control_work}")
    message(FATAL_ERROR "provider profile self-test scratch became a symlink: ${control_work}")
  endif()
  file(REAL_PATH "${control_work}" control_work_real)
  cmake_path(IS_PREFIX work "${control_work_real}" NORMALIZE control_work_inside)
  if(NOT control_work_inside OR NOT EXISTS "${control_marker}")
    message(FATAL_ERROR "provider profile self-test scratch lost its ownership marker")
  endif()
  file(READ "${control_marker}" marker_text)
  if(NOT marker_text STREQUAL "Nesturbator provider profile controls v1\n")
    message(FATAL_ERROR "provider profile self-test scratch marker changed")
  endif()
  file(REMOVE_RECURSE "${control_work_real}")
endfunction()

if(MODE STREQUAL "selftest")
  foreach(required IN ITEMS BUILD_DIR WORK)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
      message(FATAL_ERROR "provider_profile selftest requires ${required}")
    endif()
  endforeach()
  file(REAL_PATH "${BUILD_DIR}" build_root)
  if(NOT IS_DIRECTORY "${build_root}")
    message(FATAL_ERROR "provider_profile selftest requires an existing BUILD_DIR")
  endif()
  if(IS_SYMLINK "${WORK}")
    message(FATAL_ERROR "provider_profile selftest refuses a symlink WORK path")
  endif()
  file(REAL_PATH "${WORK}" work_path BASE_DIRECTORY "${build_root}")
  cmake_path(IS_PREFIX build_root "${work_path}" NORMALIZE work_inside_build)
  if(NOT work_inside_build OR work_path STREQUAL build_root)
    message(FATAL_ERROR "provider_profile selftest WORK must be below BUILD_DIR")
  endif()
  if(EXISTS "${work_path}" AND NOT IS_DIRECTORY "${work_path}")
    message(FATAL_ERROR "provider_profile selftest WORK is not a directory")
  endif()
  file(MAKE_DIRECTORY "${work_path}")

  string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef preserve_id)
  set(preserve_path "${work_path}/.provider-profile-preserve-${preserve_id}")
  if(EXISTS "${preserve_path}" OR IS_SYMLINK "${preserve_path}")
    message(FATAL_ERROR "provider profile self-test preservation control already exists")
  endif()
  set(preserve_text "caller work contents must survive profile controls\n")
  file(WRITE "${preserve_path}" "${preserve_text}")
  provider_profile_selftest("${SOURCE_DIR}" "${PROFILE}" "${work_path}")
  if(NOT EXISTS "${preserve_path}")
    message(FATAL_ERROR "provider profile self-test removed caller-owned WORK contents")
  endif()
  file(READ "${preserve_path}" actual_preserve_text)
  if(NOT actual_preserve_text STREQUAL preserve_text)
    message(FATAL_ERROR "provider profile self-test changed caller-owned WORK contents")
  endif()
  file(REMOVE "${preserve_path}")
  message(STATUS "provider profile controls passed")
  return()
elseif(NOT MODE STREQUAL "contract")
  message(FATAL_ERROR "provider_profile MODE must be contract or selftest")
endif()

provider_validate("${SOURCE_DIR}" "${PROFILE}" "${RUNTIME_REPORT}" profile_error)
if(profile_error)
  message(FATAL_ERROR "provider profile contract failed: ${profile_error}")
endif()
message(STATUS "provider profile contract passed")
