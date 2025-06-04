#include "Editor Script Utils.h"

#include <string>
#include <algorithm>

namespace Utils
{

    void ScriptReflectionGenerator::Generate(const std::filesystem::path& absolute_file_path_engine_api)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_ScriptsDir)) 
        {
            if (Louron::Utils::NormalisePath(entry.path().parent_path()).string().find("Scripts/Binaries")          != std::string::npos ||
                Louron::Utils::NormalisePath(entry.path().parent_path()).string().find("Scripts/Script Core API")   != std::string::npos ||
                Louron::Utils::NormalisePath(entry.path().parent_path()).string().find("Scripts/Generated")         != std::string::npos) 
            {
                continue;
            }

            if (entry.path().extension() == ".h") 
            {
				L_APP_INFO("Parsing Header File: {}", entry.path().string());
                current_header_file_path = entry.path();

                ParseHeader(entry.path().string(), absolute_file_path_engine_api);                
            }
        }

        WriteReflectionFile();
    }
    
    bool ScriptReflectionGenerator::HasExposedAttribute(CXCursor cursor)
    {
        CXTranslationUnit tu = clang_Cursor_getTranslationUnit(cursor);
        CXSourceLocation loc = clang_getCursorLocation(cursor);

        unsigned line, col;
        CXFile file;
        clang_getSpellingLocation(loc, &file, &line, &col, nullptr);

        // Check two lines: the same line as the field, and the one above it
        for (int i = -1; i <= 0; ++i) {
            unsigned targetLine = line + i;
            if (targetLine == 0) continue;

            CXSourceLocation start = clang_getLocation(tu, file, targetLine, 1);
            CXSourceLocation end = clang_getLocation(tu, file, targetLine, 255);
            CXSourceRange range = clang_getRange(start, end);

            CXToken* tokens = nullptr;
            unsigned numTokens;
            clang_tokenize(tu, range, &tokens, &numTokens);

            for (unsigned j = 0; j < numTokens; ++j) {
                CXString spelling = clang_getTokenSpelling(tu, tokens[j]);
                std::string token = clang_getCString(spelling);
                clang_disposeString(spelling);

                if (token == "[[ExposedInEditor]]" || token == "ExposedInEditor") 
                {
                    clang_disposeTokens(tu, tokens, numTokens);
                    return true;
                }
            }

            clang_disposeTokens(tu, tokens, numTokens);
        }

        return false;
    }

    CXChildVisitResult ScriptReflectionGenerator::ClassVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        ScriptReflectionGenerator* self = static_cast<ScriptReflectionGenerator*>(clientData);

        if (clang_getCursorKind(cursor) == CXCursor_CXXBaseSpecifier) 
        {
            CXType baseType = clang_getCanonicalType(clang_getCursorType(cursor));
            CXString baseName = clang_getTypeSpelling(baseType);
            if (std::string(clang_getCString(baseName)).find("IScript") != std::string::npos)
            {
                self->m_InheritsFromIScript = true;
            }
            clang_disposeString(baseName);
        }

        if (clang_getCursorKind(cursor) == CXCursor_FieldDecl && self->HasExposedAttribute(cursor)) 
        {
            CXString fieldName = clang_getCursorSpelling(cursor);
            CXType fieldType = clang_getCursorType(cursor);
            CXString typeName = clang_getTypeSpelling(fieldType);

            FieldInfo field;
            field.name = clang_getCString(fieldName);
            field.type = clang_getCString(typeName);
            
            std::string full_type = field.type;
            size_t pos = full_type.find_last_of(':');

            if (pos != std::string::npos && pos + 1 < full_type.size())
                field.type = full_type.substr(pos + 1);
            else
                field.type = full_type;

            self->m_CurrentClass->exposed_fields.push_back(field);

            clang_disposeString(fieldName);
            clang_disposeString(typeName);
        }

        return CXChildVisit_Recurse;
    }

    CXChildVisitResult ScriptReflectionGenerator::TUVisitor(CXCursor cursor, CXCursor parent, CXClientData clientData)
    {
        ScriptReflectionGenerator* self = static_cast<ScriptReflectionGenerator*>(clientData);

        if (clang_getCursorKind(cursor) == CXCursor_ClassDecl) {
            CXString classNameStr = clang_getCursorSpelling(cursor);
            std::string className = clang_getCString(classNameStr);
            clang_disposeString(classNameStr);

            if (className.empty()) return CXChildVisit_Continue;

            self->m_InheritsFromIScript = false;
            ScriptClassInfo classInfo;
            classInfo.name = className;
            self->m_CurrentClass = &classInfo;

            clang_visitChildren(cursor, ClassVisitor, self);

            if (self->m_InheritsFromIScript)
            {
                classInfo.file_path = self->current_header_file_path;
                self->m_ParsedScripts.push_back(classInfo);
            }
        }

        return CXChildVisit_Recurse;
    }

    void ScriptReflectionGenerator::ParseHeader(const std::filesystem::path& path, const std::filesystem::path& absolute_file_path_engine_api)
    {
    #if defined(L_PLATFORM_WINDOWS)

        const char* args[] = {
            "-x", "c++",
            "-std=c++20"
        };

    #elif defined(L_PLATFORM_LINUX)

        const char* args[] = {
            "-x", "c++",
            "-std=c++20",
            "-I", "/usr/include",
            "-I", "/usr/include/c++/13",
            "-I", "/usr/lib/gcc/x86_64-linux-gnu/13/include"
        };

    #endif

        // Read original file content
        std::ifstream file(path);
        if (!file)
        {
            L_APP_ERROR("Failed to Open File: {}", path.string());
            return;
        }

        std::stringstream original_content;
        std::string line;
        bool injected = false;
        while (std::getline(file, line))
        {
            if(line.find("Script Core API") != std::string::npos || line.find("ScriptAPI.h") != std::string::npos)
                continue;
                
            original_content << line << '\n';
            
            if(!injected)
            {
                original_content << "#include \"" << Louron::Utils::NormalisePath(absolute_file_path_engine_api).string() << "/ScriptAPI.h\"\n";
                injected = true;
            }
        }

        // Write the modified content to a temporary file
        std::filesystem::path temp_path = std::filesystem::temp_directory_path() / "temp_script_reflection.cpp";
        std::ofstream temp_file(temp_path);
        temp_file << original_content.str();
        temp_file.close();

        CXIndex index = clang_createIndex(0, 0);
        CXTranslationUnit tu = clang_parseTranslationUnit(
            index,
            temp_path.string().c_str(),
            args,
            sizeof(args) / sizeof(args[0]),
            nullptr,
            0,
            CXTranslationUnit_SkipFunctionBodies |
            CXTranslationUnit_KeepGoing
        );

        if (!tu) {
            L_APP_ERROR("Failed to Parse: {}", path.string());
            clang_disposeIndex(index);
            return;
        }

        CXCursor root = clang_getTranslationUnitCursor(tu);
        clang_visitChildren(root, TUVisitor, this);

        clang_disposeTranslationUnit(tu);
        clang_disposeIndex(index);

        if (std::filesystem::exists(temp_path))
            std::filesystem::remove(temp_path);
    }

    void ScriptReflectionGenerator::WriteReflectionFile()
    {
        std::ofstream out(m_OutputFile);
        out << "#include \"../Script Core API/ScriptAPI.h\"\n";

        for (const auto& script : m_ParsedScripts) {
            out << "#include \"../" << std::filesystem::relative(script.file_path, m_ScriptsDir).string() << "\"\n";
        }
        out << "using namespace BackEndAPI;\n";

        out << "\nextern void RegisterScript(const char* name, std::vector<FieldInfo> fields, void* (*factory)(uint32_t), void(*release)(uint32_t));\n";
        out << "extern void FinalizeScriptRegistry();\n\n";

        for (const auto& script : m_ParsedScripts)
        {
            // Create function
            out << "void* Create_" << script.name << "(uint32_t entity_uuid) {\n";
            out << "    s_ScriptInstanceMap[std::to_string(entity_uuid) + \"" << script.name << "\"] = std::make_unique<" << script.name << ">();\n";
            out << "    return s_ScriptInstanceMap[std::to_string(entity_uuid) + \"" << script.name << "\"].get();\n";
            out << "}\n";

            // Release function
            out << "void Release_" << script.name << "(uint32_t entity_uuid) {\n";
            out << "    if (auto found_instance = s_ScriptInstanceMap.find(std::to_string(entity_uuid) + \"" << script.name << "\"); found_instance != s_ScriptInstanceMap.end())\n";
            out << "        s_ScriptInstanceMap.erase(found_instance);\n";
            out << "}\n";
        }

        out << "\nSCRIPT_API void LoadScripts() {\n";

        for (const auto& script : m_ParsedScripts) {
            out << "    {\n";
            out << "        std::vector<FieldInfo> fields;\n";
            for (const auto& field : script.exposed_fields) {
                out << "        fields.push_back({ \"" << field.name << "\", offsetof(" << script.name << ", " << field.name << "), " 
                    << ToFieldTypeEnum(field.type) << " });\n";
            }
            out << "        RegisterScript(\"" << script.name << "\", std::move(fields), &Create_" << script.name << ", &Release_" << script.name << ");\n";
            out << "    }\n";
        }

        out << "    FinalizeScriptRegistry();\n";
        out << "}\n";
    }

    std::string ScriptReflectionGenerator::ToFieldTypeEnum(const std::string& type_str)
    {
        static const std::unordered_map<std::string, std::string> s_TypeMap = {
            // Custom Types
            {"vector2", "FieldType::Vector2"},
            {"vector3", "FieldType::Vector3"},
            {"vector4", "FieldType::Vector4"},
            {"uvector2", "FieldType::UVector2"},
            {"uvector3", "FieldType::UVector3"},
            {"uvector4", "FieldType::UVector4"},
            {"ivector2", "FieldType::IVector2"},
            {"ivector3", "FieldType::IVector3"},
            {"ivector4", "FieldType::IVector4"},
            {"dvector2", "FieldType::DVector2"},
            {"dvector3", "FieldType::DVector3"},
            {"bvector2", "FieldType::BVector2"},
            {"bvector3", "FieldType::BVector3"},
            {"bvector4", "FieldType::BVector4"},
            {"bvector4", "FieldType::DVector4"}, // possible typo?
            {"mat3", "FieldType::Mat3"},
            {"mat4", "FieldType::Mat4"},

            // ECS Types
            {"entity", "FieldType::Entity"},
            {"idcomponent", "FieldType::IDComponent"},
            {"tagcomponent", "FieldType::TagComponent"},
            {"hierarchycomponent", "FieldType::HierarchyComponent"},
            {"scriptcomponent", "FieldType::ScriptComponent"},
            {"transformcomponent", "FieldType::TransformComponent"},
            {"cameracomponent", "FieldType::CameraComponent"},
            {"audiolistenercomponent", "FieldType::AudioListenerComponent"},
            {"audioemittercomponent", "FieldType::AudioEmitterComponent"},
            {"meshfiltercomponent", "FieldType::MeshFilterComponent"},
            {"meshrenderercomponent", "FieldType::MeshRendererComponent"},
            {"lodmeshcomponent", "FieldType::LODMeshComponent"},
            {"skinnedmeshcomponent", "FieldType::SkinnedMeshComponent"},
            {"basicanimationcomponent", "FieldType::BasicAnimationComponent"},
            {"animatorcomponent", "FieldType::AnimatorComponent"},
            {"skyboxcomponent", "FieldType::SkyboxComponent"},
            {"pointlightcomponent", "FieldType::PointLightComponent"},
            {"spotlightcomponent", "FieldType::SpotLightComponent"},
            {"directionallightcomponent", "FieldType::DirectionalLightComponent"},
            {"rigidbodycomponent", "FieldType::RigidbodyComponent"},
            {"boxcollidercomponent", "FieldType::BoxColliderComponent"},
            {"spherecollidercomponent", "FieldType::SphereColliderComponent"},
            {"component", "FieldType::Component"},

            // Asset Types
            {"prefab", "FieldType::Prefab"},
            {"shader", "FieldType::Shader"},
            {"computeshader", "FieldType::ComputeShader"},
            {"material", "FieldType::Material"},
            {"texture2d", "FieldType::Texture2D"},
            {"texturecubemap", "FieldType::TextureCubeMap"},
            {"staticmesh", "FieldType::StaticMesh"},
            {"audioclip", "FieldType::AudioClip"},
            {"skeleton", "FieldType::Skeleton"},
            {"animationclip", "FieldType::AnimationClip"},
            {"animatormachine", "FieldType::StateMachine"},
            {"humanoid", "FieldType::Humanoid"},
            {"humanoidmask", "FieldType::HumanoidMask"},

            // Primitive Types
            {"float", "FieldType::Float"},
            {"double", "FieldType::Double"},
            {"bool", "FieldType::Bool"},

            {"int8_t", "FieldType::Int8"},
            {"int16_t", "FieldType::Int16"},
            {"int32_t", "FieldType::Int32"},
            {"int64_t", "FieldType::Int64"},
            {"uint8_t", "FieldType::UInt8"},
            {"uint16_t", "FieldType::UInt16"},
            {"uint32_t", "FieldType::UInt32"},
            {"uint64_t", "FieldType::UInt64"},

            {"char", "FieldType::Int8"},
            {"signedchar", "FieldType::Int8"},
            {"unsignedchar", "FieldType::UInt8"},
            {"short", "FieldType::Int16"},
            {"shortint", "FieldType::Int16"},
            {"signedshort", "FieldType::Int16"},
            {"signedshortint", "FieldType::Int16"},
            {"unsignedshort", "FieldType::UInt16"},
            {"unsignedshortint", "FieldType::UInt16"},
            {"int", "FieldType::Int32"},
            {"signedint", "FieldType::Int32"},
            {"signed", "FieldType::Int32"},
            {"unsigned", "FieldType::UInt32"},
            {"unsignedint", "FieldType::UInt32"},
            {"longlong", "FieldType::Int64"},
            {"longlongint", "FieldType::Int64"},
            {"signedlonglong", "FieldType::Int64"},
            {"unsignedlonglong", "FieldType::UInt64"},
            {"unsignedlonglongint", "FieldType::UInt64"},
        };

        // Normalise input
        std::string type = type_str;
        std::erase(type, ' ');
        std::transform(type.begin(), type.end(), type.begin(), [](unsigned char c) {
            return std::tolower(c);
            });

        // Special Cases
        if (type == "constchar*") return "FieldType::CString";
        if (type.find('*') != std::string::npos || type.find('&') != std::string::npos) return "FieldType::Unknown";
        if (type.starts_with("const")) type = type.substr(5);

        // Platform-specific long types
#if defined(_WIN32) || defined(_WIN64)
        if (type == "long")         return "FieldType::Int32";
        if (type == "unsignedlong") return "FieldType::UInt32";
#else
        if (type == "long")         return "FieldType::Int64";
        if (type == "unsignedlong") return "FieldType::UInt64";
#endif

        // Lookup map
        auto it = s_TypeMap.find(type);
        if (it != s_TypeMap.end())
            return it->second;

        return "FieldType::Unknown";
    }

}
