#include "../include/JsonHelper.h"
#include <sstream>
#include <cctype>

std::string JsonHelper::escapeString(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

static std::string extractStringValue(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";

    pos += needle.length();
    // Skip whitespace, colon, and newlines
    while (pos < json.length() && (isspace(json[pos]) || json[pos] == ':')) {
        pos++;
    }
    if (pos >= json.length()) return "";

    if (json[pos] == '"') {
        pos++;
        size_t endPos = json.find('"', pos);
        if (endPos == std::string::npos) return "";
        return json.substr(pos, endPos - pos);
    } else {
        // Numeric or unquoted value
        size_t endPos = pos;
        while (endPos < json.length() && json[endPos] != ',' && json[endPos] != '}' && json[endPos] != ']' && !isspace(json[endPos])) {
            endPos++;
        }
        return json.substr(pos, endPos - pos);
    }
}

JsonHelper::SolveRequest JsonHelper::parseSolveRequest(const std::string& json) {
    SolveRequest req;
    req.rows = 0;
    req.vars = 0;
    req.mode = SolveMode::RREF;
    req.parseSuccess = false;

    // Extract rows
    std::string rowsStr = extractStringValue(json, "rows");
    if (!rowsStr.empty()) {
        try { req.rows = std::stoul(rowsStr); } catch (...) {}
    }

    // Extract cols or vars
    std::string varsStr = extractStringValue(json, "vars");
    if (varsStr.empty()) varsStr = extractStringValue(json, "cols");
    if (!varsStr.empty()) {
        try { req.vars = std::stoul(varsStr); } catch (...) {}
    }

    // Extract mode
    std::string modeStr = extractStringValue(json, "mode");
    if (modeStr == "REF") {
        req.mode = SolveMode::REF;
    } else {
        req.mode = SolveMode::RREF;
    }

    // Find "equations" array
    size_t eqPos = json.find("\"equations\"");
    if (eqPos == std::string::npos) {
        req.errorMessage = "Missing 'equations' array in request.";
        return req;
    }

    size_t startArray = json.find('[', eqPos);
    if (startArray == std::string::npos) {
        req.errorMessage = "Malformed 'equations' array.";
        return req;
    }

    // Parse each equation object: {"coefficients": [...], "rhs": "..."}
    size_t curr = startArray + 1;
    while (curr < json.length()) {
        size_t objStart = json.find('{', curr);
        if (objStart == std::string::npos) break;

        size_t objEnd = objStart + 1;
        int depth = 1;
        while (objEnd < json.length() && depth > 0) {
            if (json[objEnd] == '{') depth++;
            else if (json[objEnd] == '}') depth--;
            objEnd++;
        }
        if (depth != 0) break;

        std::string eqObj = json.substr(objStart, objEnd - objStart);

        EquationInput eq;
        eq.rhs = extractStringValue(eqObj, "rhs");

        // Parse coefficients array inside eqObj
        size_t coeffPos = eqObj.find("\"coefficients\"");
        if (coeffPos != std::string::npos) {
            size_t cArrStart = eqObj.find('[', coeffPos);
            size_t cArrEnd = eqObj.find(']', cArrStart);
            if (cArrStart != std::string::npos && cArrEnd != std::string::npos) {
                std::string cArr = eqObj.substr(cArrStart + 1, cArrEnd - cArrStart - 1);
                size_t p = 0;
                while (p < cArr.length()) {
                    while (p < cArr.length() && (isspace(cArr[p]) || cArr[p] == ',')) p++;
                    if (p >= cArr.length()) break;

                    if (cArr[p] == '"') {
                        p++;
                        size_t q = cArr.find('"', p);
                        if (q == std::string::npos) break;
                        eq.coefficients.push_back(cArr.substr(p, q - p));
                        p = q + 1;
                    } else {
                        size_t q = p;
                        while (q < cArr.length() && cArr[q] != ',' && !isspace(cArr[q])) q++;
                        eq.coefficients.push_back(cArr.substr(p, q - p));
                        p = q;
                    }
                }
            }
        }

        req.equations.push_back(eq);
        curr = objEnd;
    }

    if (req.rows == 0 && !req.equations.empty()) {
        req.rows = req.equations.size();
    }
    if (req.vars == 0 && !req.equations.empty() && !req.equations[0].coefficients.empty()) {
        req.vars = req.equations[0].coefficients.size();
    }

    req.parseSuccess = true;
    return req;
}
