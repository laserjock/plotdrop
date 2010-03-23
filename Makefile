
TARGET=plotdrop
PREFIX=/usr/local
DATADIR=$(PREFIX)/share/$(TARGET)
BINDIR=$(PREFIX)/bin

VERSION=0.5.4

OBJECTS=main.o gnuplot.o droplist.o

LIBS = `pkg-config --libs gtk+-2.0 gio-2.0 glib-2.0 libglade-2.0`
CFLAGS = -g -Wall -pedantic -std=c99 `pkg-config --cflags gtk+-2.0 gio-2.0 glib-2.0 libglade-2.0` -DDATADIR="\"$(DATADIR)\"" -DVERSION="\"$(VERSION)\""

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(CFLAGS) $(LIBS) $(OBJECTS)

clean:
	rm -f *.o $(TARGET)

install: all
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(DATADIR)
	install -s $(TARGET) $(DESTDIR)$(BINDIR)
	install -m 0644 droplist.glade $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(PREFIX)/share/applications
	install -m 0644 plotdrop.desktop $(DESTDIR)$(PREFIX)/share/applications
	install -m 0644 plotdrop.png $(DESTDIR)$(DATADIR)
	install -d $(DESTDIR)$(PREFIX)/share/pixmaps
	install -m 0644 plotdrop.png $(DESTDIR)$(PREFIX)/share/pixmaps

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET) \
	$(DESTDIR)$(DATADIR)/droplist.glade \
	$(DESTDIR)$(PREFIX)/share/pixmaps/plotdrop.png \
	$(DESTDIR)$(DATADIR)/plotdrop.png \
	$(DESTDIR)$(PREFIX)/share/applications/plotdrop.desktop
