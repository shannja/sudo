#include "core.h"

QString Core::execute(const Instruction &inst, QMap<QString, Variable> &memory) {
    switch(inst.action) {
    case CMD_CREATE: return create(inst, memory);
    case CMD_DELETE: return remove(inst, memory);
    case CMD_UPDATE: return change(inst, memory);
    default: return "[ERROR] Invalid Core action.";
    }
}

QString Core::create(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (inst.category != CMD_VARIABLE) return "[ERROR] Category not supported.";

    // 1. Naming Rules
    if (!inst.name.isEmpty() && inst.name[0].isDigit())
        return "[ERROR] Name cannot start with a digit.";
    if (reservedWords.contains(inst.name.toUpper()))
        return "[ERROR] '" + inst.name + "' is a reserved keyword.";
    if (memory.contains(inst.name))
        return "[ERROR] Variable already exists. Use UPD.";

    QString finalValue = resolveValue(inst.value, memory);

    if (finalValue.startsWith("[UNDEFINED_REF]:")) {
        return "[ERROR] Undefined variable reference: " + finalValue.mid(16);
    }

    // 2. Type Check
    QString err = validateType(inst.dataType, finalValue);
    if (!err.isEmpty()) return err;

    // 3. Commit
    QString typeStr = dataTypeMap.key(inst.dataType, "STR");
    memory[inst.name] = { finalValue, typeStr };
    return "[CREATED] " + inst.name + ".";
}

QString Core::change(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (!memory.contains(inst.name)) return "[ERROR] Undefined variable.";

    QString finalValue = resolveValue(inst.value, memory);

    if (finalValue.startsWith("[UNDEFINED_REF]:")) {
        return "[ERROR] Undefined variable reference: " + finalValue.mid(16);
    }

    Variable &v = memory[inst.name];
    DataTypes dType = dataTypeMap.value(v.type);

    // Validate the RESOLVED value, not the raw token
    QString err = validateType(dType, finalValue);
    if (!err.isEmpty()) return err;

    v.value = finalValue;
    return "[OK]";
}

QString Core::validateType(DataTypes type, const QString &val) {
    if (type == CMD_NUMBER) {
        bool ok; val.toDouble(&ok);
        if (!ok) return "[ERROR] Not a valid NUM.";
    } else if (type == CMD_BOOLEAN) {
        if (val != "true" && val != "false")
            return "[ERROR] BOOL must be true/false.";
    }
    return "";
}

// Helper to determine if a string is a variable or a literal
// Now returns a special [ERROR] prefix if a variable reference is invalid
QString Core::resolveValue(const QString &rawVal, const QMap<QString, Variable> &memory) {
    // 1. If it's in memory, return the value
    if (memory.contains(rawVal)) {
        return memory[rawVal].value;
    }

    // 2. EXCEPTION: Check if it is a boolean literal before checking if it's a variable
    if (rawVal == "true" || rawVal == "false") {
        return rawVal;
    }

    // 3. Strict Check: If it starts with a letter, it must be a variable
    if (!rawVal.isEmpty() && rawVal[0].isLetter()) {
        return "[UNDEFINED_REF]:" + rawVal;
    }

    // 4. Otherwise, it's a numeric literal or string
    return rawVal;
}

QString Core::remove(const Instruction &inst, QMap<QString, Variable> &memory) {
    QStringList failedDeletions;
    int purgedCount = 0;

    for (const auto &t : inst.arguments) {
        if (memory.contains(t.value)) {
            memory.remove(t.value);
            purgedCount++;
        } else {
            failedDeletions.append(t.value);
        }
    }

    if (!failedDeletions.isEmpty()) {
        return "[ERROR] Could not delete: " + failedDeletions.join(", ") + ". (Not found)";
    }

    return "[PURGED] " + QString::number(purgedCount) + " variables.";
}
