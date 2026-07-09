#pragma once
#include <string>
#include <functional>
#include <mutex>

namespace hsi {

// Process-wide log sink. The UI installs a callback that forwards messages
// into a QPlainTextEdit / status bar; the CLI tool installs one that prints
// to stdout. Core stages never touch Qt or iostream directly.
class Logger {
public:
    using Callback = std::function<void(const std::string& stage, const std::string& message)>;

    static void setCallback(Callback cb) {
        std::lock_guard<std::mutex> lock(mutex());
        callback() = std::move(cb);
    }

    static void log(const std::string& stage, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex());
        if (callback()) callback()(stage, message);
    }

private:
    static Callback& callback() {
        static Callback cb;
        return cb;
    }
    static std::mutex& mutex() {
        static std::mutex m;
        return m;
    }
};

} // namespace hsi
