#!/usr/bin/env python3
#
# Chocolate Doom self-documentation tool.  This works similar to javadoc
# or doxygen, but documents command line parameters and configuration
# file values, generating documentation in Unix manpage and plain text forms.
#
# Comments are read from the source code in the following form:
#
#   //!
#   // @arg <extra arguments>
#   // @category Category
#   // @platform <some platform that the parameter is specific to>
#   //
#   // Long description of the parameter
#   //
#
#   something_involving = M_CheckParm("-param");
#
# For configuration file values:
#
#   //! @begin_config_file myconfig
#
#   //!
#   // Description of the configuration file value.
#   //
#
#   CONFIG_VARIABLE_INT(my_variable,       c_variable),
#

import io
import sys
import os
import re
import getopt

TEXT_WRAP_WIDTH = 78
INCLUDE_STATEMENT_RE = re.compile("@include\s+(\S+)")
CLI_PARAMETER_LINE1_RE = re.compile('^\s*CLI_Parameter\("(.*)",')
CLI_PARAMETER_LINE2_RE = re.compile('^\s*"(.*)",')
CLI_PARAMETER_LINE3_RE = re.compile('^\s*"(.*)"\)')
IFDEF_RE = re.compile('^\s*#ifdef (.*)')
IF_RE = re.compile('^\s*(else)?\s*if\((.*)\)')


# Use appropriate stdout function for Python 2 or 3
def stdout(buf):
    if sys.version_info.major < 3:
        sys.stdout.write(buf)
    else:
        sys.stdout.buffer.write(buf)


# Find the maximum width of a parameter list (for plain text output)
def variable_list_width(params):
    w = 0
    for var in params:
        pw = len(var.name) + 5

        if var is Variable and var.args:
            pw += len(var.args)
        if pw > w:
            w = pw
    return w


def parameter_list_width(params):
    w = 0
    for param in params:
        pw = 0
        for name in param.names:
            pw += len(name) + 2
        for arg in param.args:
            pw += len(arg) + 1
        pw += 1
        if pw > w:
            w = pw
    return w


class ConfigFile:
    def __init__(self, filename):
        self.filename = filename
        self.variables = []

    def add_variable(self, variable):
        self.variables.append(variable)

    def markdown_output(self):
        result = ''

        for p in self.variables:
            result += p.markdown_output()

        return result

    def manpage_output(self):
        result = ".SH CONFIGURATION VARIABLES\n"

        for v in self.variables:
            result += ".TP\n"
            result += v.manpage_output()

        return result

    def plaintext_output(self):
        result = ""

        w = variable_list_width(self.variables)

        for p in self.variables:
            result += p.plaintext_output(w)

        result = result.rstrip() + "\n"

        return result


class Variable:
    def __lt__(self, other):
        return self.name < other.name

    def __init__(self):
        self.text = ""
        self.name = ""
        self.games = None

    def add_text(self, text):
        if len(text) <= 0:
            pass
        elif text[0] == "@":
            match = re.match('@(\S+)\s*(.*)', text)

            if not match:
                raise "Malformed option line: %s" % text

            option_type = match.group(1)
            data = match.group(2)

            if option_type == "game":
                self.games = re.split(r'\s+', data.strip())

        else:
            self.text += text + " "

    def manpage_output(self):
        result = self.name
        result = '\\fB' + result + '\\fR'
        result += "\n"

        result += re.sub('\\\\', '\\\\\\\\', self.text)
        return result

    def markdown_output(self):
        name = self.name.replace("|", "&#124;")

        text = self.text.replace("|", "&#124;")

        result = "| %s | %s |\n" % (name, text)

        # html escape
        result = result.replace("<", "&lt;")
        result = result.replace(">", "&gt;")

        return result

    def plaintext_output(self, indent):
        # Build the first line, with the argument on
        start = "  " + self.name

        # pad up to the plaintext width
        start += " " * (indent - len(start))

        # Description text
        description = self.text

        # Build the complete text for the argument
        # Split the description into words and add a word at a time
        words = [word for word in re.split('\s+', description) if word]
        maxlen = TEXT_WRAP_WIDTH - indent
        outlines = [[]]
        for word in words:
            linelen = sum(len(w) + 1 for w in outlines[-1])
            if linelen + len(word) > maxlen:
                outlines.append([])
            outlines[-1].append(word)

        linesep = "\n" + " " * indent

        return (start +
                linesep.join(" ".join(line) for line in outlines) +
                "\n\n")


def add_variable(var, line, config_file):

    # If we're only targeting a particular game, check this is one of
    # the ones we're targeting.
    if len(match_games) > 0 and var.games and len(set(match_games).intersection(set(var.games))) == 0:
        return

    # Documenting a configuration file variable?

    match = re.search('CONFIG_VARIABLE_\S+\s*\(\s*(\S+?)\),', line)

    if match:
        var.name = match.group(1)
        config_file.add_variable(var)
        return

    raise Exception(var.text)


class CLIParams:
    def __init__(self):
        self.params = []

    def add_param(self, param):
        self.params.append(param)

    # Plain text output

    def plaintext_output(self):
        result = ''
        w = parameter_list_width(self.params)

        for p in self.params:
            if p.should_show():
                result += p.plaintext_output(w)

        return result

    def markdown_output(self):
        result = ''

        for p in self.params:
            if p.should_show():
                result += p.markdown_output()

        result = result.rstrip() + "\n"

        return result

    def completion_output(self):
        result = ''
        if self.params[0].should_show():
            result += self.params[0].completion_output()
        for p in self.params[1:]:
            if p.should_show():
                result += ' ' + p.completion_output()

        result = result.rstrip()

        return result

    def manpage_output(self):
        result = ""

        for p in self.params:
            if p.should_show():
                result += ".TP\n"
                result += p.manpage_output()

        return result


class CLIParam:
    def __init__(self, name):
        self.names = []
        self.args = []
        names = re.split(' ', name)
        for s in names:
            if s.startswith('-'):
                self.names.append(s.replace(',', '').rstrip())
            else:
                self.args.append(s.rstrip())

        self.games = []
        self.platform = None
        self.eng_text = None
        self.rus_text = None

    def add_games(self, games):
        for game in games:
            self.games.append(game)
        if len(self.games) == 0:
            self.games.append('Doom')
            self.games.append('Heretic')
            self.games.append('Hexen')
            self.games.append('Strife')

    def should_show(self):
        return len(set(match_games).intersection(set(self.games))) > 0 and self.platform in match_platforms

    def manpage_output(self):
        result: str = ', '.join(self.names)
        if self.args:
            result += ' ' + ' '.join(self.args)
        result = '\\fB' + result + '\\fR'

        if self.platform == '_WIN32':
            result += '\nAvailable only on: Windows' if doc_lang == 'en' else '\nДоступно только на: Windows'

        text: str = self.rus_text if doc_lang == 'ru' else self.eng_text
        result += "\n" + re.sub("\\\\", "\\\\\\\\", text) + "\n"

        return result

    def markdown_output(self):
        name: str = ', '.join(self.names)
        if self.args:
            name += ' ' + ' '.join(self.args)
        name = name.replace("|", "&#124;")

        text: str = self.rus_text if doc_lang == 'ru' else self.eng_text
        text = text.replace("|", "&#124;")

        games = ', '.join((filter(lambda game: game in match_games, map(str.capitalize, self.games))))
        if self.platform == '_WIN32':
            games += ' (Windows only)' if doc_lang == 'ru' else ' (Только на Windows)'

        result = "| %s | %s | %s |\n" % (name, text, games)

        # html escape
        result = result.replace("<", "&lt;")
        result = result.replace(">", "&gt;")
        result = result.replace("\\'", "\"")
        result = result.replace("'", "`")

        return result

    def plaintext_output(self, indent):
        # Build the first line, with the argument on
        start: str = ' ' + ', '.join(self.names)
        if self.args:
            start += ' ' + ' '.join(self.args)

        # pad up to the plaintext width
        start += ' ' * (indent - len(start))

        # Build the description text
        description: str = self.rus_text if doc_lang == 'ru' else self.eng_text
        if self.platform == '_WIN32':
            description += '\nAvailable only on: Windows' if doc_lang == 'en' else '\nДоступно только на: Windows'

        # Build the complete text for the argument
        # Split the description into words and add a word at a time
        words = [word for word in re.split('\s+', description) if word]
        maxlen = TEXT_WRAP_WIDTH - indent
        outlines = [[]]
        for word in words:
            linelen = sum(len(w) + 1 for w in outlines[-1])
            if linelen + len(word) > maxlen:
                outlines.append([])
            outlines[-1].append(word)

        linesep = "\n" + " " * indent

        return (start +
                linesep.join(" ".join(line) for line in outlines) +
                "\n\n")

    def completion_output(self):
        return ' '.join(self.names)


class CLIEnv:
    def __init__(self):
        self.platform = None
        self.games = []

    def add_game(self, game):
        self.games.append(game)

    def set_platform(self, platform):
        self.platform = platform


config_files = {}
cli_parameters = CLIParams()


def eval_if(expr, game):
    expr = expr.replace('gt_Doom', '"gt_Doom"')
    expr = expr.replace('gt_Heretic', '"gt_Heretic"')
    expr = expr.replace('gt_Hexen', '"gt_Hexen"')
    expr = expr.replace('gt_Strife', '"gt_Strife"')
    expr = expr.replace('RD_GameType', f'"gt_{game}"')
    expr = expr.replace('||', 'or')
    return eval(expr)


def process_file(filename):

    current_config_file = None

    with io.open(filename, mode='rt', encoding='UTF-8') as f:
        var = None
        waiting_for_checkparm = False
        cli_env = None
        cli_param = None

        for line in f:
            line = line.rstrip()

            # Ignore empty lines
            if re.match('\s*$', line):
                continue

            # Parse CLI help
            if cli_env:
                # CLI_Parameter record
                cli_match_line1 = CLI_PARAMETER_LINE1_RE.match(line)
                cli_match_line2 = CLI_PARAMETER_LINE2_RE.match(line)
                cli_match_line3 = CLI_PARAMETER_LINE3_RE.match(line)
                if cli_match_line1:
                    cli_param = CLIParam(cli_match_line1.group(1))
                    cli_param.add_games(cli_env.games)
                    cli_param.platform = cli_env.platform
                if cli_param and cli_match_line2:
                    cli_param.eng_text = cli_match_line2.group(1)
                if cli_param and cli_match_line3:
                    cli_param.rus_text = cli_match_line3.group(1)
                    cli_parameters.add_param(cli_param)
                    cli_param = None

                # #ifdef
                ifdef_match = IFDEF_RE.match(line)
                if ifdef_match:
                    cli_env.set_platform(ifdef_match.group(1))

                # if
                if_match = IF_RE.match(line)
                if if_match:
                    for game in ['Doom', 'Heretic', 'Hexen', 'Strife']:
                        if eval_if(if_match.group(2), game):
                            cli_env.add_game(game)

                # End of #ifdef
                if re.match('^\s*#endif', line):
                    cli_env.set_platform(None)

                # End of if
                if re.match('^\s*}', line) and not (cli_env.games is None):
                    cli_env.games = []
                    continue

                # End of CLI help
                if re.match('^}', line):
                    cli_env = None
                continue

            # Check for the start of CLI help
            if re.match('^void M_PrintHelp\(void\)', line):
                cli_env = CLIEnv()

            # Currently reading a doc comment?
            if var:
                # End of doc comment
                if not re.match('^\s*//', line):
                    waiting_for_checkparm = True

                # The first non-empty line after the documentation comment
                # ends must contain the thing being documented.
                if waiting_for_checkparm:
                    try:
                        add_variable(var, line, current_config_file)
                    except Exception:
                        pass
                    finally:
                        var = None
                else:
                    # More documentation text
                    munged_line = re.sub('\s*//\s*', '', line, 1)
                    munged_line = re.sub('\s*$', '', munged_line)
                    var.add_text(munged_line)

            # Check for start of a doc comment
            if re.search("//!", line):
                match = re.search("@begin_config_file\s*(\S+)", line)

                if match:
                    # Beginning a configuration file
                    tagname = match.group(1)
                    current_config_file = ConfigFile(tagname)
                    config_files[tagname] = current_config_file
                else:
                    # Start of a normal comment
                    var = Variable()
                    waiting_for_checkparm = False


def print_template(template_file, content):
    with io.open(template_file, encoding='UTF-8') as f:
        for line in f:
            match = INCLUDE_STATEMENT_RE.search(line)
            if match:
                filename = match.group(1)
                filename = os.path.join(os.path.dirname(template_file),
                                        filename)
                print_template(filename, content)
            else:
                line = line.replace("@content", content).rstrip() + '\n'
                if output_file_handle:
                    of.write(line)
                else:
                    stdout(line.encode('UTF-8'))


def manpage_output(targets, template_file):
    content = ""

    for t in targets:
        content += t.manpage_output() + "\n"

    content = content.replace("-", "\\-")

    print_template(template_file, content)


def markdown_output(targets, template_file):
    content = ""

    for t in targets:
        content += t.markdown_output() + "\n"

    print_template(template_file, content)


def plaintext_output(targets, template_file):
    content = ""

    for t in targets:
        content += t.plaintext_output() + "\n"

    print_template(template_file, content)


def completion_output(targets, template_file):
    content = ""

    for t in targets:
        content += t.completion_output()

    print_template(template_file, content)


def usage():
    print("Usage: %s [-c <tag>] [-g <game>] [-d <defines>] [-l <lang>] [-o <file>] ( -M | -m | -p) <file>" % sys.argv[0])
    print("   -c :  Provide documentation for the specified configuration file")
    print("         (matches the given tag name in the source file)")
    print("   -g :  Only document options for specified games (List of games should be separated by ';')")
    print("   -d :  Only document options for specified platforms described by C defines")
    print("         (List of platforms should be separated by ',')")
    print("   -l :  Output CLI documentation in specified language")
    print("   -M :  Markdown output")
    print("   -m :  Manpage output")
    print("   -p :  Plaintext output")
    print("   -b :  Bash-Completion output")
    print("   -o :  Output to file")
    sys.exit(0)


# Parse command line
opts, args = getopt.getopt(sys.argv[1:], "m:M:p:b:c:g:l:d:o:")

output_function = None
output_file = None
output_file_handle = None
template = None
doc_config_file = None
doc_lang = 'en'
match_games = []
match_platforms = [None]

for opt in opts:
    if opt[0] == "-m":
        output_function = manpage_output
        template = opt[1]
    elif opt[0] == "-M":
        output_function = markdown_output
        template = opt[1]
    elif opt[0] == "-p":
        output_function = plaintext_output
        template = opt[1]
    elif opt[0] == "-b":
        output_function = completion_output
        template = opt[1]
    elif opt[0] == "-c":
        doc_config_file = opt[1]
    elif opt[0] == "-g":
        match_games = list(map(str.capitalize, opt[1].split(';')))
    elif opt[0] == "-d":
        for platform in opt[1].split(','):
            match_platforms.append(platform)
    elif opt[0] == "-l":
        doc_lang = opt[1]
    elif opt[0] == "-o":
        output_file = opt[1]

if output_function is None or len(args) != 1:
    usage()
else:
    # Process specified files

    process_file(args[0])

    # Build a list of things to document
    if doc_config_file:
        documentation_targets = [config_files[doc_config_file]]
    else:
        documentation_targets = [cli_parameters]

    # Generate the output
    if output_file:
        with io.open(output_file, mode='wt', encoding='UTF-8') as of:
            output_file_handle = of
            output_function(documentation_targets, template)
    else:
        output_function(documentation_targets, template)
