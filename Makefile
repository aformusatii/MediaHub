CXXFLAGS = $(shell pkg-config --cflags gtk+-2.0 webkit-1.0) -Icommon -Ilgremote -Ibrowser -Iatmega -Ihtpc -I../env
CXXFLAGS +=	-g -Wall

OBJS =		bin/util.o bin/Configuration.o bin/LGRemoteClient.o bin/Browser.o bin/Atmega128.o bin/HTPC.o bin/keymap.o bin/mediahub.o

LIBS =      $(shell pkg-config --libs gtk+-2.0 webkit-1.0) -lcurl -lpthread -lz -lwiringPi

TARGET =	bin/mediahub

all:	$(TARGET) clean_ojb

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

bin/mediahub.o: src/mediahub.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
bin/keymap.o: src/keymap.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
bin/HTPC.o: htpc/HTPC.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
bin/Atmega128.o: atmega/Atmega128.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
bin/Browser.o: browser/Browser.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
bin/LGRemoteClient.o: lgremote/LGRemoteClient.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
bin/Configuration.o: common/Configuration.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
bin/util.o: common/util.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
clean_ojb:
	rm -f $(OBJS)

clean: clean_ojb
	rm -f $(TARGET)
