#include "core.h"

const QMap<Commands, Core::CoreAction> Core::operations = {
    {CMD_CREATE, Core::create},
    {CMD_UPDATE, Core::change},
    {CMD_DELETE, Core::remove}
};

QString Core::execute(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (operations.contains(inst.action)) {
        return operations[inst.action](inst, memory);
    }
    return "[ERROR] Invalid Core action.";
}

QString Core::create(const Instruction &inst, QMap<QString, Variable> &memory) {
    if (inst.category != CMD_VARIABLE) return "[ERROR] Category not supported.";

    // 1. Naming Rules
    if (!inst.name.isEmpty() && inst.name[0].isDigit())
        return "[ERROR] Name cannot start with a digit.";
    if (reservedWords.contains(inst.name))
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

/**
 * @brief Helper to determine if a string is a variable or a literal.
 * Now correctly handles String Literals wrapped in quotes.
 */
QString Core::resolveValue(const QString &rawVal, const QMap<QString, Variable> &memory) {
    if (rawVal.isEmpty()) return "";

    // 1. STRING LITERAL CHECK (Highest Priority)
    // If it's wrapped in quotes, it's definitely NOT a variable name.
    if (rawVal.startsWith("\"") && rawVal.endsWith("\"")) {
        return rawVal.mid(1, rawVal.length() - 2);
    }

    // 2. BOOLEAN LITERAL CHECK
    if (rawVal.toLower() == "true" || rawVal.toLower() == "false") {
        return rawVal.toLower();
    }

    // 3. NUMERIC LITERAL CHECK
    // If it's a valid number, it's a value, not a variable.
    bool isNum;
    rawVal.toDouble(&isNum);
    if (isNum) return rawVal;

    // 4. MEMORY LOOKUP (Variable Check)
    // If we find it in the map, return the stored value.
    if (memory.contains(rawVal)) {
        return memory[rawVal].value;
    }

    // 5. UNDEFINED REFERENCE (The Fallback)
    // If it starts with a letter but wasn't in quotes or memory, it's a broken ref.
    if (rawVal[0].isLetter()) {
        return "[UNDEFINED_REF]:" + rawVal;
    }

    return rawVal;
}

QString Core::remove(const Instruction &inst, QMap<QString, Variable> &memory) {
    // 1. Check for "MEM" (Full Purge)
    // We check the first argument: DEL MEM
    if (!inst.arguments.isEmpty()) {
        QString firstArg = inst.arguments.first().value.toUpper();

        // Using your systemIdentifiersMap to stay consistent with the VM's reserved words
        if (systemIdentifiersMap.contains(firstArg) &&
            systemIdentifiersMap.value(firstArg) == CMD_VARIABLES) {

            int count = memory.size();
            memory.clear();
            return "[PURGED] Total Memory Wipe. " + QString::number(count) + " variables removed.";
        }
    }

    // 2. Standard Individual Deletion Logic
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
