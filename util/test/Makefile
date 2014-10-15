include $(ARAWAT_DEV)/config/Makefile.pre.inc

LIBS+=-l$(LIB_PREFIX)Util  
INCLUDES+=$(SMPP_INC)

MODULE_SOURCES=	\
	rvariant_test.cpp

MODULE_HEADERS=	

MSGPACK_SOURCES=	\
	msgpack_test.cpp
	
LUABIND_SOURCES= \
 	luabind_test.cpp



OBJS=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(MODULE_SOURCES)))

OBJS1=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(MSGPACK_SOURCES)))

OBJS2=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(LUABIND_SOURCES)))
	 
TARGET = $(BIN_PREFIX)RVariantTest
TARGET1 = $(BIN_PREFIX)MsgPackUtilTest
TARGET2 = $(BIN_PREFIX)LuaBindTest

lall: $(TARGET) $(TARGET1) $(TARGET2)

$(TARGET) : ${OBJS}
	$(CC) -o $(BUILD_DIR)/$(TARGET) ${OBJS} $(LIBS)
$(TARGET1) : ${OBJS1}
	$(CC) -o $(BUILD_DIR)/$(TARGET1) ${OBJS1} $(LIBS)
$(TARGET2) : ${OBJS2}
	$(CC) -o $(BUILD_DIR)/$(TARGET2) ${OBJS2} $(LIBS)


lclean:
	rm -f $(OBJS)
	rm -f $(BUILD_DIR)/$(TARGET)
	rm -f $(OBJS1)
	rm -f $(BUILD_DIR)/$(TARGET1)
	rm -f $(OBJS2)
	rm -f $(BUILD_DIR)/$(TARGET2)


ltest: $(TARGET) $(TARGET1)
	$(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET2)


linstall: $(TARGET) $(TARGET1)
	cp -f $(BUILD_DIR)/$(TARGET) $(BIN_INSTALL)/$(TARGET)
	cp -f $(BUILD_DIR)/$(TARGET) $(BIN_INSTALL)/$(TARGET1)
	cp -f $(BUILD_DIR)/$(TARGET) $(BIN_INSTALL)/$(TARGET2)

include $(ARAWAT_DEV)/config/Makefile.post.inc
