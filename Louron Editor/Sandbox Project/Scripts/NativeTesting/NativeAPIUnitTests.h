// TestScript.h

#pragma once

#include "Script Core API/ScriptAPI.h"
#include <string>
#include <vector>
#include <format>
#include <unordered_map>

using namespace Louron;
using namespace Louron::Assets;
using namespace Louron::Components;
using namespace Louron::Vectors;
using namespace Louron::Matrices;
using namespace Louron::Partitions;

// A simple assert function that logs pass or failure messages.
void Assert(bool condition, const std::string& message)
{
    if (!condition)
    {
        Debug::Log("TEST FAILED: " + message, 2); // Log as error
    }
    else
    {
        Debug::Log("TEST PASSED: " + message, 0);
    }
}

// This test script exercises all functionality registered in the Script Register.
// Since IScript inherits from Entity, the test script is attached to an entity and
// component objects added via AddComponent<T>() will automatically use this entity.
class NativeTestScript : public IScript
{
public:
    // Exposed objects that can be set in the editor (for example, a ComputeShader asset)
    [[ExposedInEditor]] ComputeShader shader;
    [[ExposedInEditor]] BoxColliderComponent bc;

    void OnCreate() override
    {
        Debug::Log("=== Starting ScriptRegister Unit Tests ===");

        // Test basic engine systems and entity functionality using 'this'
        TestDebugFunctions();
        TestTimeFunctions();
        TestInputFunctions();
        TestEntityAndTagFunctions();
        TestTransformFunctions();

        // Test component objects via AddComponent<T>() on this entity
        TestCameraComponent();
        TestMeshFilterComponent();
        TestMeshRendererComponent();
        TestLODMeshComponent();
        TestSkinnedMeshComponent();
        TestSkyboxComponent();
        TestPointLightComponent();
        TestSpotLightComponent();
        TestDirectionalLightComponent();
        TestRigidbodyComponent();
        TestBoxColliderComponent();
        TestSphereColliderComponent();

        // Test other engine objects that are created via static functions
        TestComputeShaderFunctions();
        TestComputeBufferFunctions();
        TestMaterialFunctions();
        TestMaterialUniformBlockFunctions();
        TestTexture2DFunctions();
        TestMeshFunctions();

        Debug::Log("=== All tests completed ===");
    }

private:

    //------------------------------------------------------------------------------
    // Debug Functions
    //------------------------------------------------------------------------------
    void TestDebugFunctions()
    {
        Debug::Log("Testing Debug logging functions...");
        Debug::Log("Info message", 0);
        Debug::Log("Warning message", 1);
        Debug::Log("Error message", 2);
        Debug::Log("Fatal message", 3);
        Assert(true, "Debug logging functions executed");
    }

    //------------------------------------------------------------------------------
    // Time Functions
    //------------------------------------------------------------------------------
    void TestTimeFunctions()
    {
        Debug::Log("Testing Time functions...");

        float dt = Time::GetDeltaTime();
        float fdt = Time::GetFixedDeltaTime();
        float udt = Time::GetUnscaledDeltaTime();
        float ufdt = Time::GetUnscaledFixedDeltaTime();
        float ts = Time::GetTimeScale();
        int fr = Time::GetFrameRate();
        float currTime = Time::GetCurrTime();

        Assert(dt >= 0, "DeltaTime is non-negative");
        Assert(fdt >= 0, "FixedDeltaTime is non-negative");
        Assert(udt >= 0, "UnscaledDeltaTime is non-negative");
        Assert(ufdt >= 0, "UnscaledFixedDeltaTime is non-negative");
        Assert(ts > 0, "TimeScale is positive");
        Assert(fr > 0, "FrameRate is positive");
        Assert(currTime >= 0, "Current Time is non-negative");

        // Change and verify TimeScale.
        Time::SetTimeScale(1.5f);
        Assert(Time::GetTimeScale() == 1.5f, "TimeScale set to 1.5");
        Time::SetTimeScale(1.0f); // Reset to default
    }

    //------------------------------------------------------------------------------
    // Input Functions
    //------------------------------------------------------------------------------
    void TestInputFunctions()
    {
        Debug::Log("Testing Input functions...");
        // Actual key and mouse states are dynamic; we simply call the functions.
        bool keyState = Input::GetKey(KeyCode::A);
        bool keyDown = Input::GetKeyDown(KeyCode::A);
        bool keyUp   = Input::GetKeyUp(KeyCode::A);
        bool mouseButton = Input::GetMouseButton(MouseButtonCode::Mouse_Button_Left);
        bool mouseDown = Input::GetMouseButtonDown(MouseButtonCode::Mouse_Button_Left);
        bool mouseUp   = Input::GetMouseButtonUp(MouseButtonCode::Mouse_Button_Left);
        Vector2 mousePos = Input::GetMousePosition();

        Assert(true, "Input functions executed without crash");
    }

    //------------------------------------------------------------------------------
    // Entity and Tag Component
    //------------------------------------------------------------------------------
    void TestEntityAndTagFunctions()
    {
        Debug::Log("Testing Entity and Tag functions using this entity...");
        // 'this' is an entity because IScript inherits from Entity.
        std::string originalTag = GetTag();
        SetTag("NativeTestScriptEntity");
        Assert(GetTag() == "NativeTestScriptEntity", "Entity tag set and retrieved correctly");
    }

    //------------------------------------------------------------------------------
    // Transform Functions
    //------------------------------------------------------------------------------
    void TestTransformFunctions()
    {
        Debug::Log("Testing Transform functions using this entity...");

        Transform t;
        t.position = { 10.0f, 20.0f, 30.0f };
        t.rotation = { 0.0f, 45.0f, 90.0f };
        t.scale    = { 1.0f, 1.0f, 1.0f };

        SetTransform(t);
        Transform retT = GetTransform();

        Assert(retT.position.x == 10.0f, "Transform position.x set correctly");
        Assert(retT.rotation.y == 45.0f, "Transform rotation.y set correctly");
    }

    //------------------------------------------------------------------------------
    // Camera Component
    //------------------------------------------------------------------------------
    void TestCameraComponent()
    {
        Debug::Log("Testing CameraComponent via AddComponent...");
        CameraComponent cam = AddComponent<CameraComponent>();
        cam.SetCameraDepth(1);
        Assert(cam.GetCameraDepth() == 1, "CameraComponent depth set and retrieved");
    }

    //------------------------------------------------------------------------------
    // MeshFilter Component
    //------------------------------------------------------------------------------
    void TestMeshFilterComponent()
    {
        Debug::Log("Testing MeshFilterComponent via AddComponent...");
        auto meshFilter = AddComponent<MeshFilterComponent>();

        // Create a dummy mesh asset and test get/set.
        StaticMesh dummyMesh{};
        meshFilter.SetMeshAsset(dummyMesh);
        
        StaticMesh retMesh = meshFilter.GetMeshAsset();
        Assert((uint32_t)retMesh == (uint32_t)dummyMesh, "MeshFilter asset handle set and retrieved");

        // Set and verify mesh bounds.
        Bounds_AABB bounds;
        bounds.Min = { 0, 0, 0 };
        bounds.Max = { 1, 1, 1 };
        meshFilter.SetMeshBounds(bounds);
        Bounds_AABB retBounds = meshFilter.GetMeshBounds();
        Assert(retBounds.Max.x == 1, "MeshFilter bounds set and retrieved");
    }

    //------------------------------------------------------------------------------
    // MeshRenderer Component
    //------------------------------------------------------------------------------
    void TestMeshRendererComponent()
    {
        Debug::Log("Testing MeshRendererComponent via AddComponent...");
        auto renderer = AddComponent<MeshRendererComponent>();

        renderer.SetActive(true);
        Assert(renderer.IsActive(), "MeshRenderer active flag set and retrieved");

        renderer.SetCastingShadows(false);
        Assert(renderer.IsCastingShadows() == false, "MeshRenderer shadow flag set and retrieved");

        // Test material set/get.
        Material mat{};
        renderer.SetMaterial(mat, 0);
        Material retMat = renderer.GetMaterial(0);

        Debug::Log(std::format("Created Material: {}, Returned Material: {}", (uint32_t)mat, (uint32_t)retMat));

        Assert((uint32_t)retMat == (uint32_t)mat, "MeshRenderer material set and retrieved");

        // Test uniform block enable/disable.
        renderer.EnableUniformBlock(0);
        renderer.DisableUniformBlock(0);
        renderer.EnableAllUniformBlocks();
        renderer.DisableAllUniformBlocks();
    }

    //------------------------------------------------------------------------------
    // LODMesh Component
    //------------------------------------------------------------------------------
    void TestLODMeshComponent()
    {
        Debug::Log("Testing LODMeshComponent via AddComponent...");
        auto lodMesh = AddComponent<LODMeshComponent>();

        // Create an LOD element and add this entity as its mesh renderer.
        LODMeshComponent::LODElement lodElem;
        lodElem.DistanceThresholdNormalised = 0.5f;
        lodElem.MeshRendererEntities.push_back(*this);
        std::vector<LODMeshComponent::LODElement> lodElems = { lodElem };
        lodMesh.SetLODElements(lodElems);

        auto retElems = lodMesh.GetLODElements();
        Assert(!retElems.empty(), "LODMesh elements set and retrieved");
    }

    //------------------------------------------------------------------------------
    // SkinnedMesh Component
    //------------------------------------------------------------------------------
    void TestSkinnedMeshComponent()
    {
        Debug::Log("Testing SkinnedMeshComponent via AddComponent...");
        auto skinnedMesh = AddComponent<SkinnedMeshComponent>();

        skinnedMesh.SetActive(true);
        Assert(skinnedMesh.IsActive(), "SkinnedMesh active flag set and retrieved");

        skinnedMesh.SetCastingShadows(true);
        Assert(skinnedMesh.IsCastingShadows(), "SkinnedMesh shadow flag set and retrieved");

        StaticMesh dummyMesh{};
        skinnedMesh.SetMeshAsset(dummyMesh);

        StaticMesh retMesh = skinnedMesh.GetMeshAsset();
        Assert((uint32_t)retMesh == (uint32_t)dummyMesh, "SkinnedMesh asset handle set and retrieved");

        // Set and verify a dummy bone mapping.
        std::unordered_map<uint32_t, Entity> boneMap;
        boneMap[1] = *this;
        skinnedMesh.SetBoneMapping(boneMap);
        auto retBoneMap = skinnedMesh.GetBoneMapping();
        Assert(retBoneMap.find(1) != retBoneMap.end(), "SkinnedMesh bone mapping set and retrieved");
    }

    //------------------------------------------------------------------------------
    // Skybox Component
    //------------------------------------------------------------------------------
    void TestSkyboxComponent()
    {
        Debug::Log("Testing SkyboxComponent via AddComponent...");
        auto skybox = AddComponent<SkyboxComponent>();

        SkyboxMaterial dummyMat{};
        skybox.SetSkyboxMaterialAsset(dummyMat);

        SkyboxMaterial retMat = skybox.GetSkyboxMaterialAsset();
        Assert((uint32_t)retMat == (uint32_t)dummyMat, "Skybox material asset set and retrieved");
    }

    //------------------------------------------------------------------------------
    // Point Light Component
    //------------------------------------------------------------------------------
    void TestPointLightComponent()
    {
        Debug::Log("Testing PointLightComponent via AddComponent...");
        auto pointLight = AddComponent<PointLightComponent>();

        pointLight.SetRadius(10.0f);
        Assert(pointLight.GetRadius() == 10.0f, "PointLight radius set and retrieved");

        pointLight.SetIntensity(2.0f);
        Assert(pointLight.GetIntensity() == 2.0f, "PointLight intensity set and retrieved");

        Vector4 col = { 1, 1, 1, 1 };
        pointLight.SetColour(col);
        Vector4 retCol = pointLight.GetColour();
        Assert(retCol.x == 1, "PointLight colour set and retrieved");

        pointLight.SetShadowFlag(ShadowTypeFlag::SoftShadows);
        Assert(pointLight.GetShadowFlag() == ShadowTypeFlag::SoftShadows, "PointLight shadow flag set and retrieved");
    }

    //------------------------------------------------------------------------------
    // Spot Light Component
    //------------------------------------------------------------------------------
    void TestSpotLightComponent()
    {
        Debug::Log("Testing SpotLightComponent via AddComponent...");
        auto spotLight = AddComponent<SpotLightComponent>();

        spotLight.SetRange(15.0f);
        Assert(spotLight.GetRange() == 15.0f, "SpotLight range set and retrieved");

        spotLight.SetAngle(30.0f);
        Assert(spotLight.GetAngle() == 30.0f, "SpotLight angle set and retrieved");

        spotLight.SetIntensity(3.0f);
        Assert(spotLight.GetIntensity() == 3.0f, "SpotLight intensity set and retrieved");

        Vector4 col = { 0.5f, 0.5f, 0.5f, 1.0f };
        spotLight.SetColour(col);
        Vector4 retCol = spotLight.GetColour();
        Assert(retCol.x == 0.5f, "SpotLight colour set and retrieved");

        spotLight.SetShadowFlag(ShadowTypeFlag::SoftShadows);
        Assert(spotLight.GetShadowFlag() == ShadowTypeFlag::SoftShadows, "SpotLight shadow flag set and retrieved");
    }

    //------------------------------------------------------------------------------
    // Directional Light Component
    //------------------------------------------------------------------------------
    void TestDirectionalLightComponent()
    {
        Debug::Log("Testing DirectionalLightComponent via AddComponent...");
        auto dirLight = AddComponent<DirectionalLightComponent>();

        dirLight.SetActive(true);
        Assert(dirLight.IsActive() == true, "DirectionalLight active flag set and retrieved");

        dirLight.SetIntensity(4.0f);
        Assert(dirLight.GetIntensity() == 4.0f, "DirectionalLight intensity set and retrieved");

        dirLight.SetMaxShadowDistance(100.0f);
        Assert(dirLight.GetMaxShadowDistance() == 100.0f, "DirectionalLight max shadow distance set and retrieved");

        Vector4 col = { 1, 0, 0, 1 };
        dirLight.SetColour(col);
        Vector4 retCol = dirLight.GetColour();
        Assert(retCol.x == 1, "DirectionalLight colour set and retrieved");

        dirLight.SetShadowFlag(ShadowTypeFlag::SoftShadows);

        Debug::Log(std::string{"Directional Light Shadow Flag"} + std::to_string(static_cast<uint8_t>(dirLight.GetShadowFlag())));
        Assert(dirLight.GetShadowFlag() == ShadowTypeFlag::SoftShadows, "DirectionalLight shadow flag set and retrieved");
    }

    //------------------------------------------------------------------------------
    // Rigidbody Component
    //------------------------------------------------------------------------------
    void TestRigidbodyComponent()
    {
        Debug::Log("Testing RigidbodyComponent via AddComponent...");
        auto rb = AddComponent<RigidbodyComponent>();

        rb.SetMass(5.0f);
        Assert(rb.GetMass() == 5.0f, "Rigidbody mass set and retrieved");

        rb.SetDrag(0.1f);
        Assert(rb.GetDrag() == 0.1f, "Rigidbody drag set and retrieved");

        rb.SetAngularDrag(0.05f);
        Assert(rb.GetAngularDrag() == 0.05f, "Rigidbody angular drag set and retrieved");

        rb.SetAutomaticCentreOfMass(true);
        Assert(rb.IsAutomaticCentreOfMassEnabled() == true, "Rigidbody automatic centre of mass flag set and retrieved");

        rb.SetGravity(false);
        Assert(rb.IsGravityEnabled() == false, "Rigidbody gravity flag set and retrieved");

        rb.SetKinematic(true);
        Assert(rb.IsKinematicEnabled() == true, "Rigidbody kinematic flag set and retrieved");

        // Apply force and torque (effects may be asynchronous).
        rb.ApplyForce({ 0, 10, 0 });
        rb.ApplyTorque({ 5, 0, 0 });
    }

    //------------------------------------------------------------------------------
    // BoxCollider Component
    //------------------------------------------------------------------------------
    void TestBoxColliderComponent()
    {
        Debug::Log("Testing BoxColliderComponent via AddComponent...");
        auto boxCol = AddComponent<BoxColliderComponent>();

        boxCol.SetIsTrigger(true);
        Assert(boxCol.IsTrigger() == true, "BoxCollider trigger flag set and retrieved");

        Vector3 centre = { 0, 0, 0 };
        boxCol.SetCentre(centre);
        Vector3 retCentre = boxCol.GetCentre();
        Assert(retCentre.x == 0, "BoxCollider centre set and retrieved");

        Vector3 size = { 1, 1, 1 };
        boxCol.SetSize(size);
        Vector3 retSize = boxCol.GetSize();
        Assert(retSize.x == 1, "BoxCollider size set and retrieved");
    }

    //------------------------------------------------------------------------------
    // SphereCollider Component
    //------------------------------------------------------------------------------
    void TestSphereColliderComponent()
    {
        Debug::Log("Testing SphereColliderComponent via AddComponent...");
        auto sphereCol = AddComponent<SphereColliderComponent>();

        sphereCol.SetIsTrigger(false);
        Assert(sphereCol.IsTrigger() == false, "SphereCollider trigger flag set and retrieved");

        Vector3 centre = { 1, 1, 1 };
        sphereCol.SetCentre(centre);
        Vector3 retCentre = sphereCol.GetCentre();
        Assert(retCentre.x == 1, "SphereCollider centre set and retrieved");

        sphereCol.SetRadius(2.5f);
        Assert(sphereCol.GetRadius() == 2.5f, "SphereCollider radius set and retrieved");
    }

    //------------------------------------------------------------------------------
    // ComputeShader Functions
    //------------------------------------------------------------------------------
    void TestComputeShaderFunctions()
    {
        Debug::Log("Testing ComputeShader functions via member variable...");

        // Use the exposed shader field.
        shader.SetInt("TestInt", 42);

        shader.SetFloat("TestFloat", 3.14f);

        shader.SetBool("TestBool", true);

        shader.SetVector2("TestVector2", { 1, 2 });

        shader.SetVector3("TestVector3", { 1, 2, 3 });

        shader.SetVector4("TestVector4", { 1, 2, 3, 4 });

        shader.SetMat3("TestMat3", Mat3{ {1,0,0}, {0,1,0}, {0,0,1} });

        shader.SetMat4("TestMat4", { {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} });

        // Dummy calls for setting a buffer and dispatching.
        ComputeBuffer buffer{};
        buffer.Create(10, sizeof(float));
        shader.SetBuffer(buffer, 0);
        shader.Dispatch({1, 1, 1});
    }

    //------------------------------------------------------------------------------
    // ComputeBuffer Functions
    //------------------------------------------------------------------------------
    void TestComputeBufferFunctions()
    {
        Debug::Log("Testing ComputeBuffer non-component functions...");
        // Create a ComputeBuffer instance via its public API.
        // (Assume that the constructor allocates a buffer for 10 integers.)
        ComputeBuffer buffer(10, sizeof(int));

        std::vector<int> data = { 1,2,3,4,5,6,7,8,9,10 };
        buffer.SetData(data.data(), data.size(), sizeof(int));

        std::vector<int> retrievedData(10);
        buffer.GetData(retrievedData.data(), retrievedData.size(), sizeof(int));

        shader.SetBuffer(buffer, 0);

        Assert(data == retrievedData, "ComputeBuffer instance set and retrieved data correctly");
        buffer.Release(); // Properly release the buffer resource
    }

    //------------------------------------------------------------------------------
    // Material Functions
    //------------------------------------------------------------------------------
    void TestMaterialFunctions()
    {
    }

    //------------------------------------------------------------------------------
    // MaterialUniformBlock Functions
    //------------------------------------------------------------------------------
    void TestMaterialUniformBlockFunctions()
    {
        Debug::Log("Testing MaterialUniformBlock functions...");
        // Obtain a dummy MaterialUniformBlock pointer via a MeshRendererComponent call.

        auto mat = AddComponent<MeshRendererComponent>().GetMaterial();
        MaterialUniformBlock mub = AddComponent<MeshRendererComponent>().GetUniformBlock();
        mub.SetBool("TestUniform", false);

        unsigned char tex_data[4] = { 255, 0, 255, 255 }; 
        Texture2D texture = Texture2D(tex_data, { 1, 1}, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8);
        mub.OverrideAlbedoMap(texture);
        mub.OverrideMetallicMap(texture);
        mub.OverrideNormalMap(texture);
        mub.OverrideAlbedoTint({ 1,1,1,1 });
        mub.OverrideMetallic(0.5f);
        mub.OverrideRoughness(0.3f);
        Assert(true, "MaterialUniformBlock functions executed");
    }

    //------------------------------------------------------------------------------
    // Texture2D Functions
    //------------------------------------------------------------------------------
    void TestTexture2DFunctions()
    {
        Debug::Log("Testing Texture2D functions...");
        unsigned char tex_data[4] = { 255, 0, 0, 255 }; 
        Texture2D texture = Texture2D({256, 256}, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8);
        
        std::vector<UVector4> pixels(256 * 256, {255, 0, 0, 255});

        texture.SetPixelData(reinterpret_cast<unsigned char*>(pixels.data()), pixels.size() * 4, Texture2D::TextureFormat::RED_GREEN_BLUE_ALPHA_8);
        texture.SetPixelData(pixels);
        texture.SubmitTextureChanges();
        texture.Destroy();
        Assert(true, "Texture2D functions executed");
    }

    //------------------------------------------------------------------------------
    // Mesh Functions
    //------------------------------------------------------------------------------
    void TestMeshFunctions()
    {
        Debug::Log("Testing Mesh functions...");
        StaticMesh mesh{};

        std::vector<Vector3> vertices = { {0,0,0}, {1,0,0}, {0,1,0} };
        mesh.SetVertices(vertices);

        std::vector<Vector3> normals = { {0,0,1}, {0,0,1}, {0,0,1} };
        mesh.SetNormals(normals);

        std::vector<Vector2> texCoords = { {0,0}, {1,0}, {0,1} };
        mesh.SetTextureCoords(texCoords);

        std::vector<Vector3> tangents = { {1,0,0}, {1,0,0}, {1,0,0} };
        mesh.SetTangents(tangents);

        std::vector<Vector3> bitangents = { {0,1,0}, {0,1,0}, {0,1,0} };
        mesh.SetBitangents(bitangents);

        std::vector<uint32_t> triangles = { 0, 1, 2 };
        mesh.SetTriangles(triangles);

        size_t data_size = 0;
        const float* retVertices = mesh.GetVertices(&data_size);

        Assert(data_size / 3 == vertices.size(), "Mesh vertices set and retrieved");

        mesh.Destroy();
    }
};
