# **************************************************************************** #
#    VARIABLES
# **************************************************************************** #

NAME        := webserv
BUILDLOG    := build.log
SERVERLOG   := webserv.log
RUNCONFIG   := confDefault/easy.conf
SOURCEDIR   := sources
BUILDDIR    := build

# **************************************************************************** #
#    COMMANDS
# **************************************************************************** #

RM          := rm -rf
SCREENCLEAR := printf "\033c"

# **************************************************************************** #
#    COMPILATION
# **************************************************************************** #

CC          := c++
CFLAGS      := -g -Wall -Werror -Wextra -std=c++17
CPPFLAGS    := -c -MMD -MP
DEBUGFLAGS  := -fsanitize=address
MAKEFLAGS   += --no-print-directory -j4

# **************************************************************************** #
#    VALGRIND
# **************************************************************************** #

LEAKSLOG    := leaks.log
VLGFLAGS    := --leak-check=full \
               --show-leak-kinds=all \
               --track-origins=yes \
               --track-fds=yes \
               --trace-children=yes \
               --log-file=$(LEAKSLOG) \
               --verbose \
               --quiet

# **************************************************************************** #
#    SOURCES
# **************************************************************************** #

MODULES := network \
           config \
           config/setters \
           request \
           request/handlers \
           request/states \
           response \
           response/actions \
           exception \
           utilities

SOURCES := main \
           Client \
           PollManager \
           Server \
           ServerManager \
           SessionManager \
           Socket \
           Config \
           ConfigInitializer \
           RouteDirectiveSetter \
           ServerDirectiveSetter \
           CgiHandler \
           DeleteHandler \
           GetHandler \
           PostHandler \
           ProcessTree \
           ProcessTreeBuilder \
           Request \
           ReadState \
           ParseState \
           ProcessState \
           SendState \
           ServeDirectoryListingAction \
           ServeFileAction \
           ServeRedirectAction \
           ServeQueryAction \
           Response \
           Logger \
           Signal \
           Utility \
           UrlEncoder

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
	./$(NAME) $(RUNCONFIG)

leaks: all
	valgrind $(VLGFLAGS) ./$(NAME) $(RUNCONFIG)
	$(call report_cmd, $(LEAKSLOG))

define report_cmd
	$(SCREENCLEAR)
	cat $1 | tail -n +4 | cut --complement -d' ' -f1
endef

# **************************************************************************** #
#    BUILD
# **************************************************************************** #

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@
	printf "$(V)$(B)Binary:$(T)$(Y) $@ $(T)\n"

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
	$(call delete_cmd, $(NAME), $(SERVERLOG), $(LEAKSLOG))

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
.PHONY: debug leaks run

.SILENT:
