# Component build configuration

set(app_sources
    main.c
    wifi_manager.c
    dhcp_server.c
    ip_forward.c
    dns_forwarding.c
)

idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS ../include)
