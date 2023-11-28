CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address
TARGET = IRC
SRCDIR = .
UTILDIR = ./commandUtils
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.cpp $(UTILDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR) $(OBJDIR)/$(UTILDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@$(CXX) -MM -MT $@ $(CXXFLAGS) $< > $(OBJDIR)/$*.d

-include $(DEPS)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(TARGET)

re:
	make fclean
	make all

.PHONY: all clean fclean re