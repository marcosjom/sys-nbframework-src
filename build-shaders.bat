"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /T cs_5_0 /E CSMain /O3 /Ges /Fo src\scene\shaders\NBScnRenderJob.cs_5_0.bin src\scene\shaders\NBScnRenderJob.hlsl

"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /T cs_4_0 /E CSMain /O3 /Ges /Fo src\scene\shaders\NBScnRenderJob.cs_4_0.bin src\scene\shaders\NBScnRenderJob.hlsl

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\shaders\NBScnRenderJob.cs_5_0.bin -o include\nb\scene\shaders\NBScnRenderJob_cs_5_0.h -a NBScnRenderJob_cs_5_0

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\shaders\NBScnRenderJob.cs_4_0.bin -o include\nb\scene\shaders\NBScnRenderJob_cs_4_0.h -a NBScnRenderJob_cs_4_0

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\shaders\NBScnRenderJob.hlsl -o include\nb\scene\shaders\NBScnRenderJob_hlsl.h -a NBScnRenderJob_hlsl


glslc.exe -fshader-stage=compute -mfmt=bin -std=310es --target-env=vulkan1.0 -x glsl -O -o src\scene\shaders\NBScnRenderJob.vulkan1_0_310es.bin src\scene\shaders\NBScnRenderJob.glsl

glslc.exe -fshader-stage=compute -mfmt=bin -std=430 --target-env=vulkan1.0 -x glsl -O -o src\scene\shaders\NBScnRenderJob.vulkan1_0_430.bin src\scene\shaders\NBScnRenderJob.glsl

glslc.exe -fshader-stage=compute -mfmt=bin -std=430 --target-env=opengl4.5 -x glsl -O -o src\scene\shaders\NBScnRenderJob.opengl4_5_430.bin src\scene\shaders\NBScnRenderJob.glsl

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\shaders\NBScnRenderJob.glsl -o include\nb\scene\shaders\NBScnRenderJob_glsl.h -a NBScnRenderJob_glsl

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\shaders\NBScnRenderJob.vulkan1_0_310es.bin -o include\nb\scene\shaders\NBScnRenderJob_vulkan1_0_310es.h -a NBScnRenderJob_vulkan1_0_310es

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\shaders\NBScnRenderJob.vulkan1_0_430.bin -o include\nb\scene\shaders\NBScnRenderJob_vulkan1_0_430.h -a NBScnRenderJob_vulkan1_0_430

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\shaders\NBScnRenderJob.opengl4_5_430.bin -o include\nb\scene\shaders\NBScnRenderJob_opengl4_5_430.h -a NBScnRenderJob_opengl4_5_430


