using System.IO;
using System;
using System.Linq;
using Sharpmake;

[module: Sharpmake.Include("base.sharpmake.cs")]

[Generate]
public class JonaBaseProject : Project
{
    public JonaBaseProject() :base()
    {
        FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();
        string rootDirectory = Path.Combine(fileInfo.DirectoryName, ".");
        RootPath = Util.SimplifyPath(rootDirectory);
        Console.WriteLine($"PROJECT PATH: {RootPath}");

        AddTargets(Utils.Targets);
    }

    [Configure(), ConfigurePriority(1)]
    virtual public void ConfigureAll(Configuration conf, Target target) 
    { 
        Utils.ConfigureProjectName(conf, target);
    }

    [Configure(Optimization.Debug),ConfigurePriority(2)]
    virtual public void ConfigureDebug(Configuration config, Target target)
    {
        config.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
    }

    [Configure(Optimization.Release), ConfigurePriority(3)]
    virtual public void ConfigureRelease(Configuration config, Target target)
    {
        config.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
    }
}

[Generate]
public class EngineProject : JonaBaseProject
{
	public EngineProject()
         : base()
	{

        Name = "Engine";
        SourceRootPath = @"[project.SharpmakeCsPath]/src/Engine";

        // Hlsl files are source files
        SourceFilesExtensions.Add(".hlsl");

    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "engine";

        CompileHLSL.ConfigureShaderIncludes(conf);

        // Private dependencies
        conf.AddPublicDependency<DirectXTK>(target);
        conf.AddPublicDependency<EnkiTS>(target);
        conf.AddPublicDependency<Box2D>(target);
        conf.AddPublicDependency<ImGui>(target);
        conf.AddPrivateDependency<Assimp>(target);

        conf.PrecompHeader = "stdafx.h";
        conf.PrecompSource = "stdafx.cpp";

        // Compile C++17 
        conf.Output = Configuration.OutputType.Lib;
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189"  // Unused local variables
        ));
        //conf.Options.Add(Options.Vc.General.TreatWarningsAsErrors.Enable);
        conf.LibraryFiles.AddRange(new string[] { 
            "dxgi", 
            "d2d1", 
            "WindowsCodecs", 
            "dwrite", 
            "d3d11", 
            "d3dcompiler", 
            "Propsys",
            //"XAudio2", 
            "mfplat", 
            "mfreadwrite",
            "mfuuid", 
            "XAudio2", 
            "dxguid", 
            "Winmm" }
        );

        // Add engine include path
        conf.IncludePaths.Add(@"[project.SourceRootPath]");
    }

    protected override void ExcludeOutputFiles()
    {
        base.ExcludeOutputFiles();

        CompileHLSL.ClaimAllShaderFiles(this);
    }
}

[Generate]
public class EngineTestProject : JonaBaseProject
{
    public EngineTestProject()
         : base()
    {
        Name = "EngineTest";
        SourceRootPath = @"[project.SharpmakeCsPath]/src/EngineTests";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "engine";

        // Private dependencies
        conf.AddPrivateDependency<EngineProject>(target);

        // Compile C++17 
        conf.Output = Configuration.OutputType.Dll;
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);

        // Add engine include path
        conf.IncludeSystemPaths.Add(@"[project.SharpmakeCsPath]/src/");
        conf.IncludePrivatePaths.Add(@"[project.SourceRootPath]");
    }
}




[Generate]
public class GameProject : JonaBaseProject
{
	public GameProject() : base()
	{
        Name = "Game";
        SourceRootPath = @"[project.SharpmakeCsPath]/src/Game";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "games";

        conf.AddPrivateDependency<EngineProject>(target);

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189"  // Unused local variables
        ));

        conf.Options.Add(Options.Vc.Linker.SubSystem.Console);

        conf.Output = Configuration.OutputType.Exe;


        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/src/");
        conf.IncludePaths.Add(@"[project.SourceRootPath]");
    }
}

[Generate]
public class EngineTestBed : JonaBaseProject
{
    public EngineTestBed() : base()
    {
        Name = "EngineTestBed";
        SourceRootPath = @"[project.SharpmakeCsPath]/src/EngineTestBed";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "games";

        CompileHLSL.ConfigureShaderIncludes(conf);

        conf.AddPrivateDependency<EngineProject>(target);

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189"  // Unused local variables
        ));

        conf.Options.Add(Options.Vc.Linker.SubSystem.Console);
        conf.Output = Configuration.OutputType.Exe;

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/src/");
        conf.IncludePaths.Add(@"[project.SourceRootPath]");
    }
}




[Generate]
public class EnkiTS : ExternalProject
{
    public EnkiTS()
    {
        Name = "EnkiTS";
        SourceRootPath = Path.Combine(externalDir, "enkiTS/");

    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(Path.Combine(externalDir, "enkiTS/src"));
        conf.Output = Configuration.OutputType.Lib;
    }
}

[Generate]
public class GameSolution : Solution
{
    public GameSolution()
        :base()
    {
        // The name of the solution.
        Name = "ElectronicJonaJoy";

        // As with the project, define which target this solution builds for.
        // It's usually the same thing.
        AddTargets(Utils.Targets);
    }

    // Configure for all 4 generated targets. Note that the type of the
    // configuration object is of type Solution.Configuration this time.
    // (Instead of Project.Configuration.)
    [Configure]
    public void ConfigureAll(Solution.Configuration conf, Target target)
    {
        // Puts the generated solution in the /generated folder too.
        conf.SolutionPath = @"[solution.SharpmakeCsPath]/generated";
        conf.SolutionFileName = "[solution.Name]_[target.DevEnv]_[target.Platform]";

        // Engine project
        conf.AddProject<EngineProject>(target);

        // Test projects
        conf.AddProject<EngineTestProject>(target);

        // Game projects
        conf.AddProject<EngineTestBed>(target);
        conf.AddProject<GameProject>(target);

    }
}

[Export]
public class DirectXTK : VCPKG
{
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.LibraryFiles.Add(@"DirectXTK.lib");
    }
}

[Export]
public class Box2D : VCPKG
{
    public Box2D() : base(false)
    {
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        conf.LibraryFiles.Add(@"box2d.lib");
    }
}


[Export]
public class FreeType : VCPKG
{
    public FreeType() : base()
    {
    }
    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
    }

    public override void ConfigureRelease(Configuration conf, Target target)
    {
        base.ConfigureRelease(conf, target);
        conf.LibraryFiles.Add(@"freetype.lib");
    }

    public override void ConfigureDebug(Configuration conf, Target target)
    {
        base.ConfigureDebug(conf, target);
        conf.LibraryFiles.Add(@"freetyped.lib");
    }
}

[Export]
public class Assimp : VCPKG
{
    public override void ConfigureRelease(Configuration conf, Target target)
    {
        base.ConfigureRelease(conf, target);
        conf.LibraryFiles.Add(@"assimp-vc142-mt");
    }

    public override void ConfigureDebug(Configuration conf, Target target)
    {
        base.ConfigureDebug(conf, target);
        conf.LibraryFiles.Add(@"assimp-vc142-mtd");
    }

}


[Generate]
public class ImGui : ExternalProject
{
    public ImGui() : base()
    {
        Name = "ImGui";
        SourceRootPath = Path.Combine(externalDir, "imgui/");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        // FreeType is a dependency
        conf.AddPrivateDependency<FreeType>(target,DependencySetting.DefaultWithoutLinking);

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]");
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]/examples");
    }
}

public static class Main
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Arguments sharpmakeArgs)
    {
        // Tells Sharpmake to generate the solution described by
        // BasicsSolution.
        sharpmakeArgs.Generate<GameSolution>();
    }
}