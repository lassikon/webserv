# **************************************************************************** #
#    VARIABLES
# **************************************************************************** #

NAME 		:=	webserv
BUILDLOG	:=	build.log
SOURCEDIR	:=	sources
BUILDDIR	:=	build
DEPENDDIR	:=	.deps

# **************************************************************************** #
#    COMMANDS
# **************************************************************************** #

RM			:=	rm -rf
SCREENCLEAR	:=	printf "\033c"

# **************************************************************************** #
#    COMPILATION
# **************************************************************************** #

CC			:=	c++
CFLAGS		:=	-Wall -Werror -Wextra
DBGFLAGS	=	-g -fsanitize=address
DEPFLAGS	=	-c -MT $$@ -MMD -MP -MF $(DEPENDDIR)/$$*.d

# **************************************************************************** #
#    SOURCES
# **************************************************************************** #

MODULES :=	utils

SOURCES :=	main \
			Logger

SOURCES	:= $(addsuffix .cpp, $(SOURCES))
OBJECTS := $(addprefix $(BUILDDIR)/, $(SOURCES:.cpp=.o))
DEPENDS := $(addprefix $(DEPENDDIR)/, $(SOURCES:.cpp=.d))

SOURCEDIR += $(addprefix $(SOURCEDIR)/, $(MODULES))

vpath %.cpp $(SOURCEDIR)

# **************************************************************************** #
#    RULES
# **************************************************************************** #

all: $(NAME)

re: fclean all

debug: CFLAGS += $(DBGFLAGS)
debug: re

run: all
	$(SCREENCLEAR)
	@./$(NAME)

# **************************************************************************** #
#    BUILD
# **************************************************************************** #

$(NAME): $(OBJECTS)
	@$(CC) $(CFLAGS) $^ -o $@
	@printf "$(C)$(B)Binary:$(T)$(Y) $@\n"

define build_cmd
$1/%.o: %.cpp | $(BUILDDIR) $(DEPENDDIR)
	@if ! $(CC) $(CFLAGS) $(DEPFLAGS) $$< -o $$@ 2> $(BUILDLOG); then \
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

define clean_cmd
	@printf "$(R)$(B)Delete: $(T)$(Y)$1$(T)\n"
	@$(RM) $(NAME)
endef

clean:
	@printf "$(R)$(B)Delete: $(T)$(Y)$(DEPENDDIR) $(BUILDDIR) $(BUILDLOG)$(T)\n"
	@$(RM) $(DEPENDDIR) $(BUILDDIR)  $(BUILDLOG)

fclean: clean
	@printf "$(R)$(B)Delete: $(T)$(Y)$(NAME)$(T)\n"
	@$(RM) $(NAME)

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

$(BUILDDIR) $(DEPENDDIR):
	@mkdir -p $@

$(DEPENDS):
	include $(wildcard $(DEPENDS))

$(foreach build, $(BUILDDIR), $(eval $(call build_cmd, $(build))))

# **************************************************************************** #
#    PHONY
# **************************************************************************** #

.PHONY: all re
.PHONY: clean fclean
.PHONY: debug run