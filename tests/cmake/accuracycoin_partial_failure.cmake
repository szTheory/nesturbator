cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED OUTPUT_DIR OR NOT DEFINED PAGE OR NOT DEFINED KEY OR NOT DEFINED ADDRESS)
  message(FATAL_ERROR "partial child fixture needs OUTPUT_DIR, PAGE, KEY and ADDRESS")
endif()

file(WRITE "${OUTPUT_DIR}/raw.tsv"
  "stage\tpage\trow\tkey\tresult_address\tobserved\traw\n"
  "before-start\t${PAGE}\t1\t${KEY}\t${ADDRESS}\t0\t-\n")
file(WRITE "${OUTPUT_DIR}/diagnostics.tsv"
  "stage\tframe\tmenu_ready\tpage_index\tcursor\tautomate_test_suite\trunning_all_tests\tresult_transition\n")
message(FATAL_ERROR "controlled child failure after partial evidence")
