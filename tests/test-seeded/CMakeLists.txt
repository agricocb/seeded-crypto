package_add_test(test-derivation-options test-derivation-options.cpp lib-seeded)

target_include_directories(
    test-derivation-options
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-seeded
        ${PROJECT_SOURCE_DIR}/extern/libsodium/src/libsodium/include
)

package_add_test(test-crypto test-crypto.cpp lib-seeded)

target_include_directories(
    test-crypto
        PRIVATE
        ${PROJECT_SOURCE_DIR}/lib-seeded
        ${PROJECT_SOURCE_DIR}/extern/libsodium/src/libsodium/include
)

