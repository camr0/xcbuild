/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <xcsdk/Configuration.h>
#include <xcsdk/Environment.h>
#include <xcsdk/SDK/Manager.h>
#include <xcsdk/SDK/Toolchain.h>
#include <libutil/DefaultFilesystem.h>
#include <libutil/Filesystem.h>
#include <libutil/FSUtil.h>
#include <libutil/Options.h>
#include <libutil/Subprocess.h>
#include <libutil/SysUtil.h>
#include <pbxsetting/Type.h>

using libutil::DefaultFilesystem;
using libutil::Filesystem;
using libutil::FSUtil;
using libutil::SysUtil;
using libutil::Subprocess;

class Options {
private:
    ext::optional<bool>        _help;
    ext::optional<bool>        _version;

private:
    ext::optional<bool>        _run;
    ext::optional<bool>        _find;

private:
    ext::optional<bool>        _showSDKPath;
    ext::optional<bool>        _showSDKVersion;
    ext::optional<bool>        _showSDKBuildVersion;
    ext::optional<bool>        _showSDKPlatformPath;
    ext::optional<bool>        _showSDKPlatformVersion;

private:
    ext::optional<bool>        _log;
    ext::optional<bool>        _verbose;

private:
    ext::optional<bool>        _noCache;
    ext::optional<bool>        _killCache;

private:
    ext::optional<std::string> _toolchain;
    ext::optional<std::string> _SDK;

private:
    ext::optional<bool>        _separator;
    ext::optional<std::string> _tool;
    std::vector<std::string>   _args;

public:
    Options();
    ~Options();

public:
    bool help() const
    { return _help.value_or(false); }
    bool version() const
    { return _version.value_or(false); }

public:
    bool run() const
    { return _run.value_or(false); }
    bool find() const
    { return _find.value_or(false); }

public:
    bool showSDKPath() const
    { return _showSDKPath.value_or(false); }
    bool showSDKVersion() const
    { return _showSDKVersion.value_or(false); }
    bool showSDKBuildVersion() const
    { return _showSDKBuildVersion.value_or(false); }
    bool showSDKPlatformPath() const
    { return _showSDKPlatformPath.value_or(false); }
    bool showSDKPlatformVersion() const
    { return _showSDKPlatformVersion.value_or(false); }

public:
    bool log() const
    { return _log.value_or(false); }
    bool verbose() const
    { return _verbose.value_or(false); }

public:
    bool noCache() const
    { return _noCache.value_or(false); }
    bool killCache() const
    { return _killCache.value_or(false); }

public:
    ext::optional<std::string> const &SDK() const
    { return _SDK; }
    ext::optional<std::string> const &toolchain() const
    { return _toolchain; }

public:
    ext::optional<std::string> const &tool() const
    { return _tool; }
    std::vector<std::string> const &args() const
    { return _args; }

private:
    friend class libutil::Options;
    std::pair<bool, std::string>
    parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it);
};

Options::
Options()
{
}

Options::
~Options()
{
}

std::pair<bool, std::string> Options::
parseArgument(std::vector<std::string> const &args, std::vector<std::string>::const_iterator *it)
{
    std::string const &arg = **it;

    if (!_separator) {
        if (arg == "-h" || arg == "--help" || arg == "-help") {
            return libutil::Options::Current<bool>(&_help, arg, it);
        } else if (arg == "--version" || arg == "-version") {
            return libutil::Options::Current<bool>(&_version, arg, it);
        } else if (arg == "-r" || arg == "--run" || arg == "-run") {
            return libutil::Options::Current<bool>(&_run, arg, it);
        } else if (arg == "-f" || arg == "--find" || arg == "-find") {
            return libutil::Options::Current<bool>(&_find, arg, it);
        } else if (arg == "--show-sdk-path" || arg == "-show-sdk-path") {
            return libutil::Options::Current<bool>(&_showSDKPath, arg, it);
        } else if (arg == "--show-sdk-version" || arg == "-show-sdk-version") {
            return libutil::Options::Current<bool>(&_showSDKVersion, arg, it);
        } else if (arg == "--show-sdk-build-version" || arg == "-show-sdk-build-version") {
            return libutil::Options::Current<bool>(&_showSDKBuildVersion, arg, it);
        } else if (arg == "--show-sdk-platform-path" || arg == "-show-sdk-platform-path") {
            return libutil::Options::Current<bool>(&_showSDKPlatformPath, arg, it);
        } else if (arg == "--show-sdk-platform-version" || arg == "-show-sdk-platform-version") {
            return libutil::Options::Current<bool>(&_showSDKPlatformVersion, arg, it);
        } else if (arg == "-l" || arg == "--log" || arg == "-log") {
            return libutil::Options::Current<bool>(&_log, arg, it);
        } else if (arg == "-v" || arg == "--verbose" || arg == "-verbose") {
            return libutil::Options::Current<bool>(&_verbose, arg, it);
        } else if (arg == "-n" || arg == "--no-cache" || arg == "-no-cache") {
            return libutil::Options::Current<bool>(&_noCache, arg, it);
        } else if (arg == "-k" || arg == "--kill-cache" || arg == "-kill-cache") {
            return libutil::Options::Current<bool>(&_killCache, arg, it);
        } else if (arg == "--sdk" || arg == "-sdk") {
            return libutil::Options::Next<std::string>(&_SDK, args, it);
        } else if (arg == "--toolchain" || arg == "-toolchain") {
            return libutil::Options::Next<std::string>(&_toolchain, args, it);
        } else if (arg == "--") {
            return libutil::Options::Current<bool>(&_separator, arg, it);
        }
    }

    if (_separator || _tool || (!arg.empty() && arg[0] != '-')) {
        if (!_tool) {
            _tool = arg;
            return std::make_pair(true, std::string());
        } else {
            _args.push_back(arg);
            return std::make_pair(true, std::string());
        }
    } else {
        return std::make_pair(false, "unknown argument " + arg);
    }
}

static int
Help(std::string const &error = std::string())
{
    if (!error.empty()) {
        fprintf(stderr, "error: %s\n", error.c_str());
        fprintf(stderr, "\n");
    }

    fprintf(stderr, "Usage: xcrun [options] -- [tool] [arguments]\n\n");
    fprintf(stderr, "Find and execute developer tools.\n\n");

#define INDENT "  "
    fprintf(stderr, "Modes:\n");
    fprintf(stderr, INDENT "-r, --run (default)\n");
    fprintf(stderr, INDENT "-f, --find\n");
    fprintf(stderr, INDENT "-h, --help (this message)\n");
    fprintf(stderr, INDENT "--version\n");
    fprintf(stderr, INDENT "--show-sdk-path\n");
    fprintf(stderr, INDENT "--show-sdk-version\n");
    fprintf(stderr, INDENT "--show-sdk-build-version\n");
    fprintf(stderr, INDENT "--show-sdk-platform-path\n");
    fprintf(stderr, INDENT "--show-sdk-platform-version\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "Options:\n");
    fprintf(stderr, INDENT "-v, --verbose\n");
    fprintf(stderr, INDENT "-l, --log\n");
    fprintf(stderr, INDENT "-n, --no-cache (not implemented)\n");
    fprintf(stderr, INDENT "-k, --kill-cache (not implemented)\n");
#undef INDENT

    return (error.empty() ? 0 : -1);
}

static int
Version()
{
    printf("xcrun version 1 (xcbuild)\n");
    return 0;
}

int
main(int argc, char **argv)
{
    std::vector<std::string> args = std::vector<std::string>(argv + 1, argv + argc);

    /*
     * Parse out the options, or print help & exit.
     */
    Options options;
    std::pair<bool, std::string> result = libutil::Options::Parse<Options>(&options, args);
    if (!result.first) {
        return Help(result.second);
    }

    /*
     * Handle the basic options that don't need SDKs.
     */
    if (!options.tool()) {
        if (options.help()) {
            return Help();
        } else if (options.version()) {
            return Version();
        }
    }

    /*
     * Parse fallback options from the environment.
     */
    ext::optional<std::string> toolchainsInput = options.toolchain();
    if (!toolchainsInput) {
        if (char const *toolchains = getenv("TOOLCHAINS")) {
            toolchainsInput = std::string(toolchains);
        }
    }
    ext::optional<std::string> SDK = options.SDK();
    if (!SDK) {
        if (char const *sdkroot = getenv("SDKROOT")) {
            SDK = std::string(sdkroot);
        }
    }
    bool verbose = options.verbose() || getenv("xcrun_verbose") != NULL;
    bool log = options.log() || getenv("xcrun_log") != NULL;
    bool nocache = options.noCache() || getenv("xcrun_nocache") != NULL;

    /*
     * Warn about unhandled arguments.
     */
    if (nocache || options.killCache()) {
        fprintf(stderr, "warning: cache options not implemented\n");
    }

    /*
     * Create filesystem.
     */
    DefaultFilesystem filesystem = DefaultFilesystem();

    /*
     * Load the SDK manager from the developer root.
     */
    ext::optional<std::string> developerRoot = xcsdk::Environment::DeveloperRoot(&filesystem);
    if (!developerRoot) {
        fprintf(stderr, "error: unable to find developer root\n");
        return -1;
    }
    auto configuration = xcsdk::Configuration::Load(&filesystem, xcsdk::Configuration::DefaultPaths());
    auto manager = xcsdk::SDK::Manager::Open(&filesystem, *developerRoot, configuration);
    if (manager == nullptr) {
        fprintf(stderr, "error: unable to load manager from '%s'\n", developerRoot->c_str());
        return -1;
    }
    if (verbose) {
        fprintf(stderr, "verbose: using developer root '%s'\n", manager->path().c_str());
    }

    /*
     * Determine the SDK to use.
     */
    xcsdk::SDK::Target::shared_ptr target = manager->findTarget(SDK.value_or("macosx"));
    if (target == nullptr) {
        if (SDK) {
            fprintf(stderr, "error: unable to find sdk '%s'\n", SDK->c_str());
        } else {
            fprintf(stderr, "error: unable to find default sdk\n");
        }
        return -1;
    }
    if (verbose) {
        fprintf(stderr, "verbose: using sdk '%s': %s\n", target->canonicalName().c_str(), target->path().c_str());
    }

    /*
     * Determine the toolchains to use. Default to the SDK's toolchains.
     */
    xcsdk::SDK::Toolchain::vector toolchains;
    if (toolchainsInput) {
        /* If the custom toolchain exists, use it instead. */
        std::vector<std::string> toolchainTokens = pbxsetting::Type::ParseList(*toolchainsInput);
        for (std::string const &toolchainToken : toolchainTokens) {
            if (auto TC = manager->findToolchain(toolchainToken)) {
                toolchains.push_back(TC);
            }
        }

        if (toolchains.empty()) {
            fprintf(stderr, "error: unable to find toolchains in '%s'\n", toolchainsInput->c_str());
            return -1;
        }
    } else {
        toolchains = target->toolchains();
    }
    if (toolchains.empty()) {
        fprintf(stderr, "error: unable to find any toolchains\n");
        return -1;
    }
    if (verbose) {
        fprintf(stderr, "verbose: using toolchain(s):");
        for (xcsdk::SDK::Toolchain::shared_ptr const &toolchain : toolchains) {
            fprintf(stderr, " '%s'", toolchain->identifier().c_str());
        }
        fprintf(stderr, "\n");
    }

    /*
     * Perform actions.
     */
    if (options.showSDKPath()) {
        printf("%s\n", target->path().c_str());
        return 0;
    } else if (options.showSDKVersion()) {
        printf("%s\n", target->version().c_str());
        return 0;
    } else if (options.showSDKBuildVersion()) {
        if (auto product = target->product()) {
            printf("%s\n", product->buildVersion().c_str());
            return 0;
        } else {
            fprintf(stderr, "error: sdk has no build version\n");
            return -1;
        }
    } else if (options.showSDKPlatformPath()) {
        if (auto platform = target->platform()) {
            printf("%s\n", platform->path().c_str());
        } else {
            fprintf(stderr, "error: sdk has no platform\n");
            return -1;
        }
    } else if (options.showSDKPlatformVersion()) {
        if (auto platform = target->platform()) {
            printf("%s\n", platform->version().c_str());
        } else {
            fprintf(stderr, "error: sdk has no platform\n");
            return -1;
        }
    } else {
        if (!options.tool()) {
            return Help("no tool provided");
        }

        /*
         * Collect search paths for the tool. Can be in toolchains, target, developer root, or default paths.
         */
        std::vector<std::string> executablePaths = target->executablePaths(toolchains);
        std::vector<std::string> defaultExecutablePaths = FSUtil::GetExecutablePaths();
        executablePaths.insert(executablePaths.end(), defaultExecutablePaths.begin(), defaultExecutablePaths.end());

        /*
         * Find the tool to execute.
         */
        ext::optional<std::string> executable = filesystem.findExecutable(*options.tool(), executablePaths);
        if (!executable) {
            fprintf(stderr, "error: tool '%s' not found\n", options.tool()->c_str());
            return 1;
        }
        if (verbose) {
            fprintf(stderr, "verbose: resolved tool '%s' to: %s\n", options.tool()->c_str(), executable->c_str());
        }

        if (options.find()) {
            /*
             * Just find the tool; i.e. print its path.
             */
            printf("%s\n", executable->c_str());
            return 0;
        } else {
            /* Run is the default. */

            /*
             * Update effective environment to include the target path.
             */
            std::unordered_map<std::string, std::string> environment = SysUtil::EnvironmentVariables();
            environment["SDKROOT"] = target->path();

            if (log) {
                printf("env SDKROOT=%s %s\n", target->path().c_str(), executable->c_str());
            }

            /*
             * Execute the process!
             */
            if (verbose) {
                printf("verbose: executing tool: %s\n", executable->c_str());
            }
            Subprocess process;
            if (!process.execute(&filesystem, *executable, options.args(), environment)) {
                fprintf(stderr, "error: unable to execute tool '%s'\n", options.tool()->c_str());
                return -1;
            }

            return process.exitcode();
        }
    }
}
