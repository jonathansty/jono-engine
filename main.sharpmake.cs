using System.IO;
using System;
using Sharpmake;


public class Utils
{
    public static Target[] Targets
    {
        get
        {
            return new Target[]{
                new Target(
                  Platform.win64,
                  DevEnv.vs2019,
                  Optimization.Debug | Optimization.Release,
                  OutputType.Lib),
            };
        }
    }

    public static void ConfigureProjectName(Project.Configuration conf, Target target)
    {
        conf.ProjectFileName = "[project.Name]_[target.DevEnv]_[target.Platform]";
        conf.ProjectPath = @"[project.SharpmakeCsPath]\generated\projects";
        conf.IntermediatePath = @"[project.SharpmakeCsPath]\build\[project.Name]_[target.DevEnv]_[target.Platform]";
    }
}

[Generate]
public class JonaBaseProject : Project
{
    public JonaBaseProject() :base()
    {
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
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        // Private dependencies
        conf.AddPublicDependency<DirectXTK>(target);
        conf.AddPublicDependency<EnkiTS>(target);
        conf.AddPublicDependency<Box2D>(target);
        conf.AddPublicDependency<ImGui>(target);

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

        conf.LibraryFiles.AddRange(new string[] { "dxgi", "d2d1", "WindowsCodecs", "dwrite", "d3d11", "d3dcompiler", "Propsys","XAudio2", "mfplat", "mfreadwrite","mfuuid", "XAudio2" });


        // Add engine include path
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]");
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

        conf.AddPrivateDependency<EngineProject>(target);

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189"  // Unused local variables
        ));

        conf.Options.Add(Options.Vc.Linker.SubSystem.Application);

        conf.Output = Configuration.OutputType.Exe;


        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/src/");
        conf.IncludePaths.Add(@"[project.SourceRootPath]");
    }



}



[Generate]
public class EnkiTS : JonaBaseProject
{
    public EnkiTS()
    {
        Name = "EnkiTS";
        SourceRootPath = @"[project.SharpmakeCsPath]/external/enkiTS/src";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/external/enkiTS/src");
        conf.Output = Configuration.OutputType.Lib;
    }
}

[Generate]
public class GameSolution : Solution
{
    public GameSolution()
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

        conf.AddProject<EngineProject>(target);
        conf.AddProject<EngineTestProject>(target);
        conf.AddProject<GameProject>(target);

    }
}

[Export]
public class VCPKG : Project
{
    public string VcpkgDir { get; private set; }
    public VCPKG(bool bStatic = false) : base()
    {
        VcpkgDir = System.Environment.GetEnvironmentVariable("VCPKGDIR") + (bStatic ? "-static" : "");
        AddTargets(Utils.Targets);
    }

    [Configure(), ConfigurePriority(1)]
    public virtual void ConfigureAll(Configuration conf, Target target)
    {
        conf.Output = Configuration.OutputType.Lib;
    }

    [Configure(Optimization.Release), ConfigurePriority(3)]
    public virtual void ConfigureRelease(Configuration conf, Target target) 
    {
        conf.IncludePaths.Add(Path.Combine(VcpkgDir, "include"));
        conf.LibraryPaths.Add(Path.Combine(VcpkgDir, "lib"));
    }

    [Configure(Optimization.Debug), ConfigurePriority(2)]
    public virtual void ConfigureDebug(Configuration conf, Target target) 
    {
        conf.IncludePaths.Add(Path.Combine(VcpkgDir, "include"));
        conf.LibraryPaths.Add(Path.Combine(VcpkgDir, "debug/lib"));
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



public class ExternalProject : JonaBaseProject
{
    protected string externalDir = @"[project.SharpmakeCsPath]/external";






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