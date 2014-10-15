SUBDIRS=smpp_sim test

include $(ARAWAT_DEV)/config/Makefile.pre.inc

LIBS+=-l$(LIB_PREFIX)Util 
INCLUDES +=-Iinclude

MODULE_SOURCES=	\
	smpp_conn.cpp

MODULE_HEADERS=	\
	smpp_conn.hpp	\
	smpp_config.hpp	


OBJS=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(MODULE_SOURCES)))
	 
TARGET_LIB = lib$(LIB_PREFIX)Smpp.so

lall: $(TARGET_LIB)

$(TARGET_LIB) : ${OBJS}
	$(CC) -shared -Wl,-soname,$(TARGET_LIB) -o $(BUILD_DIR)/$(TARGET_LIB) ${OBJS} $(LIBS)


lclean:
	rm -f $(OBJS)
	rm -f $(BUILD_DIR)/$(TARGET_LIB)

ltest: 

linstall: $(TARGET_LIB)
	cp -f $(BUILD_DIR)/$(TARGET_LIB) $(LIB_INSTALL)/$(TARGET_LIB)


include $(ARAWAT_DEV)/config/Makefile.post.inc
