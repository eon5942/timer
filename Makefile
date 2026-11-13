PREFIX ?= /usr/local
CXX ?= c++
CXXFLAGS ?= -O2

timer: timer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

install: timer
	install -Dm755 timer $(DESTDIR)$(PREFIX)/bin/timer

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/timer

clean:
	rm -f timer

.PHONY: install uninstall clean
