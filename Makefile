CXX = g++
# COmmpilation Options
CXXFLAGS = -std=c++11 -Wall -Wextra

# sources files
SOURCES = DSHKTelemetryDecoder.cpp NasaDSTelemetryDecodingTool.cpp

OBJECTS = $(SOURCES:.cpp=.o)
# exe
EXECUTABLE = NasaDSTelemetryDecodingTool

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXECUTABLE)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compilation clean
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
