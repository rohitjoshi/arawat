include $(ARAWAT_DEV)/config/Makefile.pre.inc

LIBS+=-l$(LIB_PREFIX)Util 
INCLUDES+=$(QPID_INC) $(SMPP_INC)

MODULE_SOURCES=	\
	qpid_sender_test.cpp
	
MODULE_SOURCES1=	\
	qpid_receiver_test.cpp

MODULE_HEADERS=	


OBJS=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(MODULE_SOURCES)))
	 
OBJS1=$(addprefix $(BUILD_DIR)/, \
	 $(patsubst %.cpp, %.o, $(MODULE_SOURCES1)))

TARGET = $(BIN_PREFIX)QpidSenderTest
TARGET1 = $(BIN_PREFIX)QpidReceiverTest

lall: $(TARGET) $(TARGET1)
$(TARGET) : ${OBJS}
	$(CC) -o $(BUILD_DIR)/$(TARGET) ${OBJS} $(LIBS)

$(TARGET1) : ${OBJS1}
	$(CC) -o $(BUILD_DIR)/$(TARGET1) ${OBJS1} $(LIBS)

lclean:
	rm -f $(OBJS)
	rm -f $(OBJS1)
	rm -f $(BUILD_DIR)/$(TARGET)
	rm -f $(BUILD_DIR)/$(TARGET1)


ltest: $(TARGET) $(TARGET1)
	$(BUILD_DIR)/$(TARGET)
	$(BUILD_DIR)/$(TARGET1)


linstall: $(TARGET) $(TARGET)
	cp -f $(BUILD_DIR)/$(TARGET) $(BIN_INSTALL)/$(TARGET)
	cp -f $(BUILD_DIR)/$(TARGET1) $(BIN_INSTALL)/$(TARGET1)

include $(ARAWAT_DEV)/config/Makefile.post.inc
