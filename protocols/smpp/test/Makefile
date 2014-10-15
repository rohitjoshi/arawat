include $(ARAWAT_DEV)/config/Makefile.pre.inc

LIBS+=-l$(LIB_PREFIX)Util -l$(LIB_PREFIX)Smpp
INCLUDES+=$(SMPP_INC)

MODULE_SOURCES=	\
	smpp_test.cpp

MODULE_HEADERS=	


OBJS=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(MODULE_SOURCES)))

TARGET = $(BIN_PREFIX)SmppTest

lall: $(TARGET)
$(TARGET) : ${OBJS}
	$(CC) -o $(BUILD_DIR)/$(TARGET) ${OBJS} $(LIBS)



lclean:
	rm -f $(OBJS)
	rm -f $(BUILD_DIR)/$(TARGET)


ltest: $(TARGET)
	$(BUILD_DIR)/$(TARGET)


linstall: $(TARGET)
	cp -f $(BUILD_DIR)/$(TARGET) $(BIN_INSTALL)/$(TARGET)

include $(ARAWAT_DEV)/config/Makefile.post.inc
