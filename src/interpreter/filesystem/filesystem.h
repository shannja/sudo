#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../../definitions.h"

class FileSystem {
public:
    typedef QString (*FileOP)(const Instruction&, QMap<QString, Variable>&);

    // Used by INFO to show filesystem tree.
    static QString getTree(const QString &startPath = "/");

    // The main entry point called by the Interpreter
    static QString execute(const Instruction &inst, QMap<QString, Variable> &memory);

    // FS Handlers
    static QString makeDir(const Instruction &inst, QMap<QString, Variable> &memory);
    static QString renameDir(const Instruction &inst, QMap<QString, Variable> &memory);
    static QString changeDir(const Instruction &inst, QMap<QString, Variable> &memory);
    static QString removeDir(const Instruction &inst, QMap<QString, Variable> &memory);

private:
    static const QMap<FileSystemOperations, FileOP> operations;

    static QString traverse(const QDir &dir, int level, QString indent);

    // Security/Path Helpers
    static QString getPhysicalPath(const QString &input);
    static void init();

    // OS State
    static QString m_rootPath;      // The actual file folder (e.g., /home/user/SUDO/ROOT)
    static QString m_virtualPath;   // The VM user's view (e.g., /Documents)
};

#endif // FILESYSTEM_H
