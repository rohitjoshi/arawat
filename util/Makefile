SUBDIRS=test


include $(ARAWAT_DEV)/config/Makefile.pre.inc
INCLUDES +=-Iinclude

MODULE_SOURCES=	\
	log.cpp	\
	boost_logger.cpp

MODULE_HEADERS=	\
	log.hpp	\
	boost_logger.hpp \
	rvariant.hpp \
	uuid.hpp \
	msgpack_util.hpp 

OBJS=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(MODULE_SOURCES)))

TARGET_LIB = lib$(LIB_PREFIX)Util.so

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
