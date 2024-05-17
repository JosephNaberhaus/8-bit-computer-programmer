idf_component_register(
    SRCS memory_write.c
    INCLUDE_DIRS .
    REQUIRES driver
    PRIV_REQUIRES byte_output freertos output)