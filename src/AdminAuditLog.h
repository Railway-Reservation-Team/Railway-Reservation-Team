#ifndef RRS_ADMIN_AUDIT_LOG_H
#define RRS_ADMIN_AUDIT_LOG_H

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace AdminAuditLog {

// Appends a timestamped administrator event without recording credentials or
// passenger personal details. The timestamp is Unix epoch seconds (UTC).
inline bool append(
    const std::string& event,
    const std::string& subject = "none") noexcept {
    try {
        std::error_code filesystemError;
        std::filesystem::create_directories("data", filesystemError);
        if (filesystemError) {
            return false;
        }

        const auto timestamp =
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

        auto safeField = [](std::string value) {
            for (char& ch : value) {
                if (ch == '|' || ch == '\n' || ch == '\r') {
                    ch = '_';
                }
            }
            return value;
        };

        std::ofstream output(
            "data/admin_audit.log",
            std::ios::out | std::ios::app
        );

        if (!output.is_open()) {
            return false;
        }

        output << timestamp << '|'
               << safeField(event) << '|'
               << safeField(subject) << '\n';
        output.flush();
        return static_cast<bool>(output);
    } catch (...) {
        return false;
    }
}

} // namespace AdminAuditLog

#endif
