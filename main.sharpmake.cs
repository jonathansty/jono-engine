using System.IO;
using System;
using System.Linq;
using Sharpmake;

[module: Sharpmake.Include("base.sharpmake.cs")]
[module: Sharpmake.Include("external.sharpmake.cs")]


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

        conf.AddPublicDependency<DirectXTK>(target);
        conf.AddPublicDependency<ImGui>(target);
        conf.AddPublicDependency<HLSLPP>(target);
        conf.AddPublicDependency<EnkiTS>(target);

        // Private dependencies
        // Only visible to the engine layer
        conf.AddPrivateDependency<Box2D>(target);
        conf.AddPrivateDependency<Assimp>(target);

        conf.PrecompHeader = "engine.stdafx.h";
        conf.PrecompSource = "engine.stdafx.cpp";

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

        Configuration.BuildStepExecutable step = new Configuration.BuildStepExecutable(
            @"[project.SharpmakeCsPath]/generated/projects/output/win64/debug/reflection-generator.exe",
            "", "", "-directory=src/engine -output=obj/reflection -root=[project.SharpmakeCsPath]");
        conf.EventPreBuildExe.Add(step);

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/obj/reflection/src/engine/");
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
        conf.AddPrivateDependency<EngineProject>(target, DependencySetting.DefaultWithoutBuildSteps);

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

        conf.AddPrivateDependency<EngineProject>(target, DependencySetting.DefaultWithoutBuildSteps);

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189"  // Unused local variables
        ));

        conf.Options.Add(Options.Vc.Linker.SubSystem.Console);

        conf.PrecompHeader = "game.stdafx.h";
        conf.PrecompSource = "game.stdafx.cpp";

        conf.Output = Configuration.OutputType.Exe;


        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/src/");
        conf.IncludePaths.Add(@"[project.SourceRootPath]");

        Configuration.BuildStepExecutable step = new Configuration.BuildStepExecutable(@"[project.SharpmakeCsPath]/generated/projects/output/win64/debug/reflection-generator.exe","", "", "-directory=src/game -output=obj/reflection -root=[project.SharpmakeCsPath]");
        conf.EventPreBuildExe.Add(step);

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
        conf.PrecompHeader = "testbed.stdafx.h";
        conf.PrecompSource = "testbed.stdafx.cpp";

        CompileHLSL.ConfigureShaderIncludes(conf);

        conf.AddPrivateDependency<EngineProject>(target, DependencySetting.DefaultWithoutBuildSteps);

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
public class ToolsProject : JonaBaseProject
{
    public ToolsProject()
    {
        SourceRootPath = @"[project.SharpmakeCsPath]/src/tools/[project.Name]";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "tools";
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
    }


}

[Generate]
public class ReflectionGenerator : ToolsProject
{
    public ReflectionGenerator() : base()
    {
        Name = "reflection-generator";
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

        conf.AddProject<ReflectionGenerator>(target);




    }
}


public static class Main
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Arguments sharpmakeArgs)
    {
        sharpmakeArgs.Builder.EventPostProjectLink += Builder_EventPostProjectLink;
        // Tells Sharpmake to generate the solution described by
        // BasicsSolution.
        sharpmakeArgs.Generate<GameSolution>();
    }

    private static void Builder_EventPostProjectLink(Project project)
    {
        if(project is JonaBaseProject && !(project is ExternalProject) && !(project is ToolsProject))
        {
            Sharpmake.Resolver resolver = new Sharpmake.Resolver();
            resolver.SetParameter("project", project);
            string outdir = @"[project.SharpmakeCsPath]\generated\reflection\dumps\[project.Name].txt";
            outdir = resolver.Resolve(outdir);

            if (!Directory.Exists(Path.GetDirectoryName(outdir)))
            {
                Directory.CreateDirectory(Path.GetDirectoryName(outdir));
            }
            using (StreamWriter file = new StreamWriter(outdir))
            {
                Console.WriteLine($"[{project.Name}] Generating file list");
                foreach (var filePath in project.SourceFiles.Where(p => p.EndsWith(".h", StringComparison.InvariantCultureIgnoreCase)))
                {
                    file.WriteLine(filePath);
                }
            }
        }
    }
}