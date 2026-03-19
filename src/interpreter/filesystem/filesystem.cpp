#include "filesystem.h"
#include "../core/core.h"

// Initialize static state
QString FileSystem::m_rootPath = "";
QString FileSystem::m_virtualPath = "/";

const QMap<FileSystemOperations, FileSystem::FileOP> FileSystem::operations = {
    {CMD_MAKE_DIRECTORY,      FileSystem::makeDir},
    {CMD_RENAME_DIRECTORY,    FileSystem::renameDir},
    {CMD_REMOVE_DIRECTORY,    FileSystem::removeDir},
    {CMD_CHANGE_DIRECTORY,    FileSystem::changeDir},
};

/**
 * @brief Ensures the ROOT directory is prepared on the Linux host.
 */
void FileSystem::init() {
    if (m_rootPath.isEmpty()) {
        m_rootPath = QDir::cleanPath(QCoreApplication::applicationDirPath() + "/ROOT");
        if (!QDir(m_rootPath).exists()) {
            QDir().mkpath(m_rootPath);
        }
    }
}

/**
 * @brief THE DISPATCHER: Routes commands to the specific handlers.
 */
QString FileSystem::execute(const Instruction &inst, QMap<QString, Variable> &memory) {
    init(); // Ensure we have a place to write files

    // Convert the string-based action to our Enum
    FileSystemOperations opType = fileSystemOperationsMap.value(inst.value, CMD_FILE_SYSTEM_INVALID);

    if (!operations.contains(opType)) {
        return "[ERROR] Unknown FL action: " + inst.value;
    }

    // Execute the mapped function
    return operations[opType](inst, memory);
}

/**
 * @brief THE SECURITY GUARD: Translates VM paths to Linux paths.
 * Blocks any attempt to use ".." to escape the ROOT folder.
 */
QString FileSystem::getPhysicalPath(const QString &input) {
    init();
    QString target = QDir::cleanPath(m_rootPath + m_virtualPath + "/" + input);

    // Sandbox Check: Does the result still live inside our ROOT?
    if (!target.startsWith(m_rootPath)) return "";
    return target;
}

/**
 * @brief Prints the file system tree.
 */
QString FileSystem::getTree(const QString &startPath) {
    // Always start from the real ROOT on the disk
    QDir rootDir(m_rootPath);
    if (!rootDir.exists()) return "[ERROR] Sandbox root missing.";

    return traverse(rootDir, 0, "");
}

/**
 * @brief Recursive function to address the file system.
 */
QString FileSystem::traverse(const QDir &dir, int level, QString indent) {
    QString result;
    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (int i = 0; i < list.size(); ++i) {
        bool isLast = (i == list.size() - 1);
        QFileInfo fileInfo = list[i];

        // 1. Draw the current line
        result += indent + (isLast ? "└── " : "├── ") + fileInfo.fileName() + "/\n";

        // 2. Prepare the indent for the next level
        // If this is the last child, we stop drawing the vertical line for this branch
        QString nextIndent = indent + (isLast ? "    " : "│   ");

        // 3. Recurse
        result += traverse(QDir(fileInfo.absoluteFilePath()), level + 1, nextIndent);
    }
    return result;
}

// --- HANDLER IMPLEMENTATIONS ---

QString FileSystem::makeDir(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (inst.arguments.isEmpty()) return "[ERROR] MKDIR: Argument required.";

    QString name = Core::resolveValue(inst.arguments[0].value, memory);
    QString path = getPhysicalPath(name);

    if (path.isEmpty()) return "[ERROR] Security: Outside sandbox.";
    if (QDir(path).exists()) return "[ERROR] Directory already exists.";

    return QDir().mkpath(path) ? "[OK] Created: " + name : "[ERROR] Kernel denied write.";
}

QString FileSystem::changeDir(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (inst.arguments.isEmpty()) return "[ERROR] CD: Target path required.";

    // Get the raw token (e.g., "DIR" or "projectRoot")
    QString rawToken = inst.arguments[0].value;

    // 1. DIRECT ROOT JUMP (Using 'DIR' as the keyword)
    if (systemIdentifiersMap.value(rawToken) == CMD_ROOT_DIRECTORY) {
        m_virtualPath = "/";
        return "[OK] Returned to (ROOT).";
    }

    // 2. Resolve for variables if it's not a system keyword
    QString target = Core::resolveValue(rawToken, memory);

    // 3. PARENT DIRECTORY JUMP
    if (target == "..") {
        if (m_virtualPath == "/") return "[OK] Already at (ROOT).";
        m_virtualPath = QDir::cleanPath(m_virtualPath + "/..");
        return "[OK] Path: " + m_virtualPath;
    }

    // 4. STANDARD NAVIGATION
    QString fullPath = getPhysicalPath(target);
    if (fullPath.isEmpty() || !QDir(fullPath).exists()) {
        return "[ERROR] Directory '" + target + "' not found.";
    }

    m_virtualPath = QDir::cleanPath(m_virtualPath + "/" + target);
    return "[OK] Path: " + m_virtualPath;
}

QString FileSystem::removeDir(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (inst.arguments.isEmpty()) return "[ERROR] RMDIR: Target required.";

    QString name = Core::resolveValue(inst.arguments[0].value, memory);
    QString path = getPhysicalPath(name);

    if (path.isEmpty()) return "[ERROR] Security: Access denied.";
    if (!QDir(path).exists()) return "[ERROR] Not found.";

    // Standard Linux behavior: only removes if empty.
    return QDir().rmdir(path) ? "[OK] Removed: " + name : "[ERROR] Folder not empty or busy.";
}

QString FileSystem::renameDir(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (inst.arguments.size() < 2) return "[ERROR] RNDIR: [OLD] [NEW] required.";

    QString oldName = Core::resolveValue(inst.arguments[0].value, memory);
    QString newName = Core::resolveValue(inst.arguments[1].value, memory);

    QString oldPath = getPhysicalPath(oldName);
    QString newPath = getPhysicalPath(newName);

    if (oldPath.isEmpty() || newPath.isEmpty()) return "[ERROR] Security breach.";

    return QDir().rename(oldPath, newPath) ? "[OK] Renamed." : "[ERROR] Rename failed.";
}
