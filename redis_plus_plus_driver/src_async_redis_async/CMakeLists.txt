cmake_minimum_required(VERSION 3.8)
project(asnc_gRPC_test)

find_package(Threads REQUIRED)

option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED)
set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

find_package(PkgConfig REQUIRED)
pkg_search_module(GRPC REQUIRED grpc)
pkg_search_module(GRPCPP REQUIRED grpc++>=1.22.0)
list(APPEND _GRPC_GRPCPP ${GRPCPP_LIBRARIES})
list(APPEND _GRPC_GRPCPP "absl::atomic_hook absl::errno_saver absl::log_severity absl::no_destructor absl::nullability absl::raw_logging_internal absl::spinlock_wait absl::config absl::dynamic_annotations absl::core_headers absl::malloc_internal absl::base_internal absl::base absl::throw_delegate absl::pretty_function absl::endian absl::scoped_set_env absl::strerror absl::fast_type_id absl::prefetch absl::algorithm absl::algorithm_container absl::cleanup_internal absl::cleanup absl::btree absl::compressed_tuple absl::fixed_array absl::inlined_vector_internal absl::inlined_vector absl::test_allocator absl::flat_hash_map absl::flat_hash_set absl::node_hash_map absl::node_hash_set absl::container_memory absl::hash_function_defaults absl::hash_policy_traits absl::common_policy_traits absl::hashtablez_sampler absl::hashtable_debug absl::hashtable_debug_hooks absl::node_slot_policy absl::raw_hash_map absl::container_common absl::raw_hash_set absl::layout absl::crc_cpu_detect absl::crc_internal absl::crc32c absl::non_temporal_arm_intrinsics absl::non_temporal_memcpy absl::crc_cord_state absl::stacktrace absl::symbolize absl::examine_stack absl::failure_signal_handler absl::debugging_internal absl::demangle_internal absl::leak_check absl::debugging absl::flags_path_util absl::flags_program_name absl::flags_config absl::flags_marshalling absl::flags_commandlineflag_internal absl::flags_commandlineflag absl::flags_private_handle_accessor absl::flags_reflection absl::flags_internal absl::flags absl::flags_usage_internal absl::flags_usage absl::flags_parse absl::any_invocable absl::bind_front absl::function_ref absl::overload absl::hash absl::city absl::low_level_hash absl::log_internal_check_impl absl::log_internal_check_op absl::log_internal_conditions absl::log_internal_config absl::log_internal_flags absl::log_internal_format absl::log_internal_globals absl::log_internal_log_impl absl::log_internal_proto absl::log_internal_message absl::log_internal_log_sink_set absl::log_internal_nullguard absl::log_internal_nullstream absl::log_internal_strip absl::log_internal_voidify absl::log_internal_append_truncated absl::absl_check absl::absl_log absl::check absl::die_if_null absl::log_flags absl::log_globals absl::log_initialize absl::log absl::log_entry absl::log_sink absl::log_sink_registry absl::log_streamer absl::log_internal_structured absl::log_structured absl::vlog_config_internal absl::absl_vlog_is_on absl::vlog_is_on absl::log_internal_fnmatch absl::memory absl::type_traits absl::meta absl::bits absl::int128 absl::numeric absl::numeric_representation absl::sample_recorder absl::exponential_biased absl::periodic_sampler absl::random_random absl::random_bit_gen_ref absl::random_internal_mock_helpers absl::random_distributions absl::random_seed_gen_exception absl::random_seed_sequences absl::random_internal_traits absl::random_internal_distribution_caller absl::random_internal_fast_uniform_bits absl::random_internal_seed_material absl::random_internal_pool_urbg absl::random_internal_salted_seed_seq absl::random_internal_iostream_state_saver absl::random_internal_generate_real absl::random_internal_wide_multiply absl::random_internal_fastmath absl::random_internal_nonsecure_base absl::random_internal_pcg_engine absl::random_internal_randen_engine absl::random_internal_platform absl::random_internal_randen absl::random_internal_randen_slow absl::random_internal_randen_hwaes absl::random_internal_randen_hwaes_impl absl::random_internal_distribution_test_util absl::random_internal_uniform_helper absl::status absl::statusor absl::string_view absl::strings absl::charset absl::has_ostream_operator absl::strings_internal absl::str_format absl::str_format_internal absl::cord_internal absl::cordz_update_tracker absl::cordz_functions absl::cordz_statistics absl::cordz_handle absl::cordz_info absl::cordz_sample_token absl::cordz_update_scope absl::cord absl::graphcycles_internal absl::kernel_timeout_internal absl::synchronization absl::time absl::civil_time absl::time_zone absl::any absl::bad_any_cast absl::bad_any_cast_impl absl::span absl::optional absl::bad_optional_access absl::bad_variant_access absl::variant absl::compare absl::utility absl::if_constexpr")
message(STATUS "Using gRPC libs: ${_GRPC_GRPCPP} ")

find_program(_PROTOBUF_PROTOC protoc)
find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)

#find_path(HIREDIS_HEADER hiredis)
#target_include_directories(target PUBLIC ${HIREDIS_HEADER})
find_library(HIREDIS_LIB hiredis)
#target_link_libraries(target ${HIREDIS_LIB})

#find_path(REDIS_PLUS_PLUS_HEADER sw)
#target_include_directories(target PUBLIC ${REDIS_PLUS_PLUS_HEADER})
find_library(REDIS_PLUS_PLUS_LIB redis++)
#target_link_libraries(target ${REDIS_PLUS_PLUS_LIB})

#set(_GRPC_GRPCPP gRPC::grpc++)


# Proto file
get_filename_component(hw_proto "../../../test1/protos/CustomerLimitStorageRPC.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
set(hw_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.pb.cc")
set(hw_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.pb.h")
set(hw_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.grpc.pb.cc")
set(hw_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/CustomerLimitStorageRPC.grpc.pb.h")
add_custom_command(
      OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${hw_proto}"
      DEPENDS "${hw_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(hw_grpc_proto
  ${hw_grpc_srcs}
  ${hw_grpc_hdrs}
  ${hw_proto_srcs}
  ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  ${HIREDIS_LIB}
  ${REDIS_PLUS_PLUS_LIB})

# Targets greeter_[async_](client|server)
foreach(_target
  sync_create async_read 
  async_server 
  async_create2)
  add_executable(${_target} "${_target}.cpp")
  target_link_libraries(${_target}
    hw_grpc_proto
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    ${HIREDIS_LIB}
    ${REDIS_PLUS_PLUS_LIB})
endforeach()

