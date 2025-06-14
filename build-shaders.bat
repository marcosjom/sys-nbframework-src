"C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\fxc.exe" /T cs_5_0 /E CSMain /O3 /Ges /Fo src\scene\NBScnRenderJob.cs_5_0.bin src\scene\NBScnRenderJob.hlsl
.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\NBScnRenderJob.hlsl -o include\nb\scene\NBScnRenderJob_hlsl.h -a NBScnRenderJob_hlsl
.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\NBScnRenderJob.cs_5_0.bin -o include\nb\scene\NBScnRenderJob_cs_5_0.h -a NBScnRenderJob_cs_5_0


glslc.exe -fshader-stage=compute -mfmt=bin -std=310es --target-env=vulkan1.0 -x glsl -O -o src\scene\NBScnRenderJob.vulkan1_0_310es.bin src\scene\NBScnRenderJob.glsl
glslc.exe -fshader-stage=compute -mfmt=bin -std=430 --target-env=vulkan1.0 -x glsl -O -o src\scene\NBScnRenderJob.vulkan1_0_430.bin src\scene\NBScnRenderJob.glsl

glslc.exe -fshader-stage=compute -mfmt=bin -std=430 --target-env=opengl4.5 -x glsl -O -o src\scene\NBScnRenderJob.opengl4_5_430.bin src\scene\NBScnRenderJob.glsl

.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\NBScnRenderJob.glsl -o include\nb\scene\NBScnRenderJob_glsl.h -a NBScnRenderJob_glsl
.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\NBScnRenderJob.vulkan1_0_310es.bin -o include\nb\scene\NBScnRenderJob_vulkan1_0_310es.h -a NBScnRenderJob_vulkan1_0_310es
.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\NBScnRenderJob.vulkan1_0_430.bin -o include\nb\scene\NBScnRenderJob_vulkan1_0_430.h -a NBScnRenderJob_vulkan1_0_430
.\bin\win\Release\x64\bin2c.exe -t char -l 20 -i src\scene\NBScnRenderJob.opengl4_5_430.bin -o include\nb\scene\NBScnRenderJob_opengl4_5_430.h -a NBScnRenderJob_opengl4_5_430


