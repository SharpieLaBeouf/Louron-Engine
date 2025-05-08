# Louron Native Script Core API

The **Louron Script Core API** is the native C++ scripting interface used to extend behavior and logic in the Louron game engine.  
This API enables seamless, real-time interaction with scene entities, engine systems, and components from within your own C++ scripts.

---

## Overview

Scripts in Louron are created and managed via the **Editor**, inside the `Scripts/` folder of your project.  
Each script is a C++ class that **inherits from `Louron::IScript`**, and is automatically compiled into a shared library that the engine loads at runtime.

All script classes are tied to an entity in the scene.

As **`Louron::IScript` inherits from `Louron::Entity`**, you can use all the methods and properties of an entity in your script.

---

## IScript Class

The base interface for all user scripts is:

```cpp
class MyScript : public Louron::IScript
{
    void OnCreate() override { }
    void OnUpdate() override { }
    void OnLateUpdate() override { }
    void OnFixedUpdate() override { }
    void OnDestroy() override { }

    // Collider Functions
    void OnCollideEnter(const Collider& other) override { }
    void OnCollideStay(const Collider& other) override { }
    void OnCollideLeave(const Collider& other) override { }

    // Collider Trigger Functions
    void OnTriggerEnter(const Collider& other) override { }
    void OnTriggerStay(const Collider& other) override { }
    void OnTriggerLeave(const Collider& other) override { }
};
```

These functions will be called automatically by the engine at the appropriate times.

All scripts must be declared in the global namespace, otherwise the engine will not be able to find them.

## Scripting

### Namespaces

It is recommended to use the following namespace for all your scripts:

```cpp
using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;
```

This makes it easier to access the Louron API without having to type the full namespace every time.

### Fields

For any fields you want to have accessible in the editor/serialisable & deserialisable, these must be marked with the `[[ExposedInEditor]]` attribute.

For example:

```cpp
class MyScript : public Louron::IScript
{
    [[ExposedInEditor]]
    int myInt = 0;

    [[ExposedInEditor]] float myFloat = 0.0f;

    bool [[ExposedInEditor]] my_bool = false;

    const char* my_c_string [[ExposedInEditor]] = "Hello World";
}
```

Types such as `std::string`, `std::wstring`, `std::vector`, `std::map`, `std::unordered_map`, smart pointers, and any other types that create issues when transferred between ABI boundaries are not supported in the editor. 

You must use pure raw types and the ones provided through the API, such as `Vector3`, if you need to access fields in the editor.

### Other Script Instances

You can access other script instances in the scene using the `ScriptComponent` attached to an entity.

It is your responsibility to check if the script instance is not `nullptr` before using it. Further, it is your responsibility to reinterpret cast the script instance to the correct type.

For example:

```cpp
class MyScript : public Louron::IScript
{
public:

    [[ExposedInEditor]] Entity other_entity; // Set in editor

    bool my_bool = false;

    void OnCreate() override
    {
        // Get the script component of the other entity
        MyScript* other_script = reinterpret_cast<MyScript*>(other_entity.GetComponent<ScriptComponent>().GetScriptInstance("MyScript"));
        
        // Check if the script instance is valid
        if (other_script)
        {
            // Use the other script instance
            other_script.my_bool = true;
        }
    }
};
```

## Compiling Script Code DLL

The editor will take care of the compilation of your script code into a shared library.

Every time you modify a script, and refocus the editor, the engine will automatically compile the script code into a shared library.

You can find the compiled shared library in the `Scripts/Generated` folder of your project.

It is not recommended to compile the script code manually, as the editor will take care of this for you.

The compiler used is clang++.exe which is distributed as a tool alongside the Game Engine.

Each time the script project is recompiled, the Script Core API in the Scripts directory will be erased and reset with the one from the engine to ensure no funny business!