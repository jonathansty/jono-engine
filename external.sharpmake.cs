using Sharpmake;
using System.IO;
using System.Collections;
using System.Collections.Generic;

[Generate]
public class ImGui : ExternalProject
{
    public ImGui() : base()
    {
        Name = "ImGui";
        SourceRootPath = Path.Combine(ExternalDir, "imgui/");
        // SourceFilesExcludeRegex.Add("examples");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.AddPublicDependency<SDL2>(target);

        conf.Output = target.OutputType == OutputType.Lib ? Configuration.OutputType.Lib : Configuration.OutputType.Dll;
        if (conf.Output == Configuration.OutputType.Dll)
        {
            string capitalisedName = conf.Project.Name.ToUpper();
            conf.Defines.Add($"{capitalisedName}_DLL");
            conf.Defines.Add($"{capitalisedName}_EXPORTS");

            conf.ExportDefines.Add($"{capitalisedName}_DLL");
        }

        conf.IncludePaths.Add(Path.Combine(ExternalDir, "SDL2/SDL2-2.24.1/include/SDL2"));
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]");
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]/examples");
    }
}

[Export]
public class Box2D : ExternalProject
{
    public Box2D() : base()
    {
        Name = "Box2D";
    }

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludeSystemPaths.Add(Path.Combine(ExternalDir, "Box2D/include/"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "Box2D/include/"));

        if(target.GetOptimization() == Optimization.Release)
        {
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "Box2D/build/bin/Release"));
        }
        else
        {
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "Box2D/build/bin/Debug"));
        }

        conf.LibraryFiles.Add("Box2D.lib");
    }


}



[Generate]
public class ImPlot : ExternalProject
{
    public ImPlot() : base()
    {
        Name = "ImPlot";
        SourceRootPath = Path.Combine(ExternalDir, "ImPlot/");
        // SourceFilesExcludeRegex.Add("examples");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.AddPrivateDependency<ImGui>(target);

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]");
    }
}


[Generate]
public class Rttr : ExternalProject
{
    public Rttr() : base()
    {
        Name = "rttr";
        SourceRootPath = Path.Combine(ExternalDir, "rttr/src/rttr/");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        if(conf.Output == Configuration.OutputType.Dll)
        {
            conf.Defines.Add("RTTR_DLL");
            conf.Defines.Add("RTTR_DLL_EXPORTS");
            conf.ExportDefines.Add("RTTR_DLL");
        }


        conf.IncludePaths.Add(Path.Combine(ExternalDir, "rttr/src"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "rttr/src/detail"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "rttr/build/src"));
    }
}


[Export]
public class Hlslpp : ExternalProject
{
    public Hlslpp() : base()
    {
        Name = "hlslpp";
        SourceRootPath = Path.Combine(ExternalDir, "hlslpp/include/");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludeSystemPaths.Add(Path.Combine(ExternalDir, "hlslpp/include/"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "hlslpp/include/"));
    }
}

[Export]
public class Assimp : ExternalProject
{
    public Assimp() : base()
    {
        Name = "assimp";
        SourceRootPath = Path.Combine(ExternalDir, "assimp/include/");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludeSystemPaths.Add(Path.Combine(ExternalDir, "assimp/include/"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "assimp/include/"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "../build/assimp/x64/include/"));

        if(target.GetOptimization() == Optimization.Release)
        {
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "../build/assimp/x64/lib/Release/"));
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "../build/assimp/x64/contrib/zlib/Release/"));
            conf.LibraryFiles.Add("zlibstatic.lib");
            conf.LibraryFiles.Add("assimp-vc143-mt.lib");
        }
        else
        {
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "../build/assimp/x64/lib/Debug/"));
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "../build/assimp/x64/contrib/zlib/Debug/"));
            conf.LibraryFiles.Add("zlibstaticd.lib");
            conf.LibraryFiles.Add("assimp-vc143-mtd.lib");
        }
    }
}

[Export]
public class FmtLib : ExternalProject
{
    public FmtLib() : base()
    {
        Name = "fmt";
        SourceRootPath = Path.Combine(ExternalDir, "fmt/include");
    }

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludeSystemPaths.Add(Path.Combine(ExternalDir, "fmt/include/"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "fmt/include/"));
        if(target.GetOptimization() == Optimization.Release)
        {
            conf.LibraryFiles.Add("fmt.lib");
            conf.LibraryPaths.Add(@"[project.SharpmakeCsPath]/build/fmt/x64/Release/");
        }
        else
        {
            conf.LibraryFiles.Add("fmtd.lib");
            conf.LibraryPaths.Add(@"[project.SharpmakeCsPath]/build/fmt/x64/Debug/");
        }
    }
}

[Generate]
public class EnkiTS : ExternalProject
{
    public EnkiTS() : base()
    {
        Name = "EnkiTS";
        SourceRootPath = Path.Combine(ExternalDir, "enkitS/src/");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludeSystemPaths.Add(Path.Combine(ExternalDir, "enkiTS/src/"));
    }
}

[Generate]
public class OpTick : ExternalProject
{
    public OpTick() : base()
    {
        Name = "OpTick";
        SourceRootPath = Path.Combine(ExternalDir, "OpTick/src");
    }

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Defines.Add("OPTICK_ENABLE_GPU=0");
        conf.Defines.Add("OPTICK_ENABLE_GPU_D3D12=0");

        conf.IncludePaths.Add(Path.Combine(ExternalDir, "optick/include"));

        if(target.OutputType == OutputType.Dll)
        {
            conf.Defines.Add("OPTICK_EXPORTS");
        }
        // conf.LibraryFiles.AddRange(new string[] { 
        //     "OpTickCore.lib"
        // });
    }

    override public void ConfigureDebug(Configuration config, Target target)
    {
        base.ConfigureDebug(config, target);
        // config.LibraryPaths.Add(Path.Combine(ExternalDir, "OpTick/lib/x64/debug"));
    }
    override public void ConfigureRelease(Configuration config, Target target)
    {
        base.ConfigureRelease(config, target);
        // config.LibraryPaths.Add(Path.Combine(ExternalDir, "OpTick/lib/x64/release"));
    }
}

[Export]
public class DirectXTK : ExternalProject
{
    public DirectXTK() : base()
    {
        Name = "DirectXTK";
        SourceRootPath = @"[project.ExternalDir]/DirectXTK/";
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.Output = Configuration.OutputType.None;
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "DirectXTK/Inc/"));
        if(target.GetOptimization() == Optimization.Debug)
        {
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "DirectXTK/Bin/Desktop_2022/x64/Debug"));
        }
        else
        {
            conf.LibraryPaths.Add(Path.Combine(ExternalDir, "DirectXTK/Bin/Desktop_2022/x64/Release"));
        }
        conf.LibraryFiles.Add("DirectXTK.lib");
    }
}





[Export]
public class  SDL2 : ExternalProject
{
    public SDL2() : base()
    {
        Name = "SDL2";
        SourceRootPath = Path.Combine(ExternalDir, "SDL2/");
    }
    
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "SDL2/SDL2-2.24.1/include"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "SDL2/SDL2-2.24.1/include/SDL"));
        conf.LibraryPaths.Add(Path.Combine(ExternalDir, "SDL2/SDL2-2.24.1/lib/x64"));
        conf.LibraryFiles.AddRange(new string[]{
            "SDL2.lib", "SDL2main.lib"
        });
        conf.TargetCopyFiles.Add(Path.Combine(ExternalDir, "SDL2/SDL2-2.24.1/lib/x64/SDL2.dll"));
        conf.TargetCopyFilesPath = conf.TargetPath;
    }
}
