# **************************************************************************** #
#    VARIABLES
# **************************************************************************** #

NAME      := webserv
BUILDLOG  := build.log
SERVERLOG := webserv.log
SOURCEDIR := sources
BUILDDIR  := build

# **************************************************************************** #
#    COMMANDS
# **************************************************************************** #

RM          := rm -rf
SCREENCLEAR := printf "\033c"

# **************************************************************************** #
#    COMPILATION
# **************************************************************************** #

CC         := c++
CFLAGS     := -g -Wall -Werror -Wextra
CPPFLAGS   := -c -MMD -MP
DEBUGFLAGS := -fsanitize=address
MAKEFLAGS  += -j4 --no-print-directory

# **************************************************************************** #
#    SOURCES
# **************************************************************************** #

MODULES := utils \
		   config 

SOURCES := main \
           Logger \
		   config \
		   utility \
		   Exception

SOURCES := $(addsuffix .cpp, $(SOURCES))
OBJECTS := $(addprefix $(BUILDDIR)/, $(SOURCES:.cpp=.o))

SOURCEDIR += $(addprefix $(SOURCEDIR)/, $(MODULES))

INCS := $(addprefix -I, $(SOURCEDIR))

DEPS := $(OBJECTS:.o=.d)

vpath %.cpp $(SOURCEDIR)

# **************************************************************************** #
#    RULES
# **************************************************************************** #

all: $(NAME)

re: fclean
	make all

debug: CFLAGS += $(DEBUGFLAGS)
debug: re

run: all
	$(SCREENCLEAR)
	./$(NAME)

# **************************************************************************** #
#    BUILD
# **************************************************************************** #

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@
	printf "$(V)$(B)Binary:$(T)$(Y) $@\n"

define build_cmd
$1/%.o: %.cpp | $(BUILDDIR)
	if ! $(CC) $(CFLAGS) $(CPPFLAGS) $(INCS) $$< -o $$@ 2> $(BUILDLOG); then \
		printf "$(R)$(B)\nError: \
		$(V)Unable to create object file: \
		$(R)$(B)$$@$(Y)\n\n"; \
		sed '$$d' $(BUILDLOG); exit 1 ; \
	else \
		printf "$(C)$(B)Object: $(G)$$@ $(T)\n"; \
	fi
endef

# **************************************************************************** #
#    CLEAN
# **************************************************************************** #

clean:
	$(call delete_cmd, $(BUILDDIR), $(BUILDLOG))

fclean: clean
	$(call delete_cmd, $(NAME), $(SERVERLOG))

define delete_cmd
	printf "$(R)$(B)Delete:$(T)$(Y)$1$2$3$4$5$(T)\n"
	$(RM) $1 $2 $3 $4 $5
endef

# **************************************************************************** #
#    COLORS
# **************************************************************************** #

T = \033[0m
B = \033[1m
G = \033[32m
V = \033[35m
C = \033[36m
Y = \033[33m
R = \033[31m

# **************************************************************************** #
#    UTILS
# **************************************************************************** #

-include $(DEPS)

$(BUILDDIR):
	mkdir -p $@

$(foreach build, $(BUILDDIR), $(eval $(call build_cmd, $(build))))

# **************************************************************************** #
#    PHONY
# **************************************************************************** #

.PHONY: all re
.PHONY: clean fclean
.PHONY: debug run

.SILENT:
