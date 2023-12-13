CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
DEBUGFLAGS = -g3 -fsanitize=address -fsanitize=undefined -fstack-protector-all -D DEBUG
TARGET = ircserv
SRCDIR = .
UTILDIR = ./commandUtils
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.cpp $(UTILDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

debug:
	make -j CXXFLAGS="$(CXXFLAGS) $(DEBUGFLAGS)" all


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