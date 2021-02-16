externalTool\Release\bin\dxc.exe -spirv -T vs_6_0 -E  main .\shaders\triangle.vert.hlsl -Fo .\shaders\triangle.vert.spv
externalTool\Release\bin\dxc.exe -spirv -T ps_6_0 -E  main .\shaders\triangle.frag.hlsl -Fo .\shaders\triangle.frag.spv
externalTool\Release\bin\dxc.exe -spirv -T vs_6_0 -E  main .\shaders\Sky.vert.hlsl -Fo .\shaders\Sky.vert.spv
externalTool\Release\bin\dxc.exe -spirv -T ps_6_0 -E  main .\shaders\Sky.frag.hlsl -Fo .\shaders\Sky.frag.spv