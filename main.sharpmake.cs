using System.IO;
using System;
using System.Linq;
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
        conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
        conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]";
    }
}

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
            "XAudio2", 
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
        conf.Output = Configuration.OutputType.None;
    }

    [Configure(Optimization.Debug), ConfigurePriority(2)]
    public virtual void ConfigureDebug(Configuration conf, Target target)
    {
        conf.IncludePaths.Add(Path.Combine(VcpkgDir, "include"));
        conf.LibraryPaths.Add(Path.Combine(VcpkgDir, "debug/lib"));
    }

    [Configure(Optimization.Release), ConfigurePriority(3)]
    public virtual void ConfigureRelease(Configuration conf, Target target) 
    {
        conf.IncludePaths.Add(Path.Combine(VcpkgDir, "include"));
        conf.LibraryPaths.Add(Path.Combine(VcpkgDir, "lib"));
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

public class CompileHLSL : Project.Configuration.CustomFileBuildStep
{
    public enum ShaderProfile
    {
        ps_5_0,
        vs_5_0,
        cs_5_0,
    }
    public CompileHLSL(Project.Configuration conf, ShaderProfile profile, string outputDir, string targetName, string filename) :base() {

        KeyInput = filename;

        string resourceName = Path.GetFileNameWithoutExtension(filename);
        Output = $"{outputDir}/{resourceName}.h";
        Executable = "fxc";

        ExecutableArguments = string.Format("/Zi /nologo /O2 /E\"{0}\" /T {1} /Fh\"{2}\" /Vn\"cso_{3}\" \"{4}\"", "main", profile.ToString(), Output, resourceName, filename);
    }


    public static void ConfigureShaderIncludes(Project.Configuration conf)
    {
        string outputDir = $"{conf.Project.RootPath}/obj/{conf.Project.Name}_{conf.Target.GetOptimization()}";
        conf.IncludePaths.Add(outputDir);

    }

    public static void ClaimAllShaderFiles(Project project)
    {
        ClaimShaderFiles(project, ShaderProfile.vs_5_0, "_vx.hlsl", "main");
        ClaimShaderFiles(project, ShaderProfile.ps_5_0, "_px.hlsl", "main");

    }
    public static void ClaimShaderFiles(Project project, ShaderProfile profile, string ext, string entryName)
    {
        Strings hlslFiles = new Strings(project.ResolvedSourceFiles.Where(file => file.EndsWith(ext, StringComparison.InvariantCultureIgnoreCase)));
        if(hlslFiles.Count() > 0)
        {
            foreach (Project.Configuration conf in project.Configurations)
            {
                string targetName = conf.Target.Name;

                foreach (string file in hlslFiles)
                {
                    string outputDir = string.Format(@"{0}\obj\{1}_{2}\shaders\", project.SharpmakeCsPath, project.Name, conf.Target.GetOptimization());
                    CompileHLSL compileTask = new CompileHLSL(conf, profile, outputDir, targetName, Project.GetCapitalizedFile(file));
                    project.ResolvedSourceFiles.Add(compileTask.Output);
                    conf.CustomFileBuildSteps.Add(compileTask);
                }

            }

        }

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