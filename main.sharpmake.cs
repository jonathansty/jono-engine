using System.IO;
using System;
using System.Linq;
using Sharpmake;

[module: Sharpmake.Include("base.sharpmake.cs")]
[module: Sharpmake.Include("external.sharpmake.cs")]


[Generate]
public class CliModule : Module
{
    public CliModule() : base()
    {
        Name = "CLI";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
    }
}

[Generate]
public class CoreModule : Module
{
    public CoreModule() : base()
    {
        Name = "Core";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.AddPublicDependency<OpTick>(target);
        conf.AddPublicDependency<Rttr>(target);
        conf.AddPublicDependency<Hlslpp>(target);
    }
}


[Generate]
public class EngineModule : Module
{
	public EngineModule()
         : base()
	{

        Name = "Engine";

        // Hlsl files are source files
        SourceFilesExtensions.Add(".hlsl");

    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        CompileHLSL.ConfigureShaderIncludes(conf);

        conf.AddPublicDependency<ImGui>(target);
        conf.AddPublicDependency<ImPlot>(target);

        // Own public libraries
        conf.AddPublicDependency<CliModule>(target);
        conf.AddPublicDependency<CoreModule>(target);

        //conf.AddPublicDependency<DirectXTK>(target);
        conf.ProjectReferencesByPath.Add(@"[project.SharpmakeCsPath]/external/DirectXTK/DirectXTK_Desktop_2022.vcxproj");
        //conf.ReferencesByName.Add("DirectXTK_Desktop_2022");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/external/DirectXTK/Inc");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/external/DirectXTK/Src");


        conf.AddPublicDependency<SDL2>(target);


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


        //conf.EventPreBuildExe.Add(ReflectionGenerator.GetCustomBuildStep());
        //conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/obj/reflection/src/engine/");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");


    }

    protected override void ExcludeOutputFiles()
    {
        base.ExcludeOutputFiles();

        // CompileHLSL.ClaimAllShaderFiles(this);
    }
}

[Generate]
public class EngineTestProject : Application
{
    public EngineTestProject()
         : base()
    {
        Name = "tests";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = "tests";

        // Private dependencies
        conf.AddPrivateDependency<EngineModule>(target, DependencySetting.DefaultWithoutBuildSteps);

        // Compile C++17 
        conf.Output = Configuration.OutputType.Dll;

        // Add engine include path
        conf.IncludeSystemPaths.Add($"[project.SharpmakeCsPath]/{Utils.SourceFolderName}");
        conf.IncludePrivatePaths.Add(@"[project.SourceRootPath]");
    }
}

[Generate]
public class SceneViewerProject : Application
{
    public SceneViewerProject() : base()
    {
        Name = "SceneViewer";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        conf.SolutionFolder = Utils.g_FilterFolderGame;

        CompileHLSL.ConfigureShaderIncludes(conf);

        conf.AddPrivateDependency<EngineModule>(target);
        conf.AddPrivateDependency<CoreModule>(target);
        conf.AddPrivateDependency<CliModule>(target);
        conf.AddPrivateDependency<ImGui>(target);
        conf.AddPrivateDependency<SDL2>(target);

        conf.Options.Add(Options.Vc.Linker.SubSystem.Console);
        conf.Output = Configuration.OutputType.Exe;

        conf.IncludePaths.Add(@"[project.SourceRootPath]");
    }
}

[Generate]
public class PathFindingProject : Application
{
    public PathFindingProject() : base()
    {
        Name = "PathFindingTest";
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        conf.Output = Configuration.OutputType.Exe;

        base.ConfigureAll(conf, target);
        conf.SolutionFolder = Utils.g_FilterFolderGame;

        CompileHLSL.ConfigureShaderIncludes(conf);

        conf.AddPrivateDependency<EngineModule>(target);

        conf.Options.Add(Options.Vc.Linker.SubSystem.Console);
        conf.Output = Configuration.OutputType.Exe;

        conf.IncludePaths.Add(@"[project.SourceRootPath]");

    }
}


[Generate]
public abstract class ToolsProject : Application
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
public class EngineSolution : Solution
{
    public EngineSolution()
        :base()
    {
        // The name of the solution.
        Name = "jono-engine";

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
        conf.SolutionPath = @"[solution.SharpmakeCsPath]/build";
        conf.SolutionFileName = "[solution.Name]_[target.DevEnv]";

        conf.Name = @"[target.Optimization] [target.OutputType]";

        conf.ProjectReferencesByPath.Add(@"[solution.SharpmakeCsPath]/external/DirectXTK/DirectXTK_Desktop_2022.vcxproj");

        conf.AddProject<EngineModule>(target);
        conf.AddProject<SceneViewerProject>(target);
        //conf.AddProject<PathFindingProject>(target);
    }
}

[Generate]
public class ToolsOnlySolution : Solution
{
    public ToolsOnlySolution()
        : base()
    {
        // The name of the solution.
        Name = "Tools";

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
        conf.SolutionPath = @"[solution.SharpmakeCsPath]/build";
        conf.SolutionFileName = "[solution.Name]_[target.DevEnv]";

        conf.AddProject<EngineModule>(target);
        conf.AddProject<SceneViewerProject>(target);
        conf.AddProject<PathFindingProject>(target);
        conf.AddProject<EngineTestProject>(target);
    }
}

[Generate]
public class TestSolution : Solution
{
    public TestSolution()
        : base()
    {
        // The name of the solution.
        Name = "Tests";

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
        conf.SolutionPath = @"[solution.SharpmakeCsPath]/build";
        conf.SolutionFileName = "[solution.Name]_[target.DevEnv]";

        // Engine project
        var types = System.Reflection.Assembly.GetExecutingAssembly()
            .GetTypes()
            .Where(type =>
            {
                return type.IsSubclassOf(typeof(ToolsProject)) && !type.IsAbstract && Attribute.GetCustomAttribute(type, typeof(Sharpmake.Generate)) != null;
            });

        foreach (var type in types)
        {
            conf.AddProject(type, target);
        }

        conf.AddProject<EngineModule>(target);
    }
}


public static class Main
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Arguments sharpmakeArgs)
    {
        // Post 'linking' we generate a list of files for tools to consume
        sharpmakeArgs.Builder.EventPostProjectLink += Builder_EventPostProjectLink;

        sharpmakeArgs.Builder.EventPreSolutionConfigure += Builder_EventPreSolutionConfigure;


        // Generate the solution for our game (all)
        sharpmakeArgs.Generate<EngineSolution>();

        // Generate just tools projects
        // sharpmakeArgs.Generate<TestSolution>();
        // sharpmakeArgs.Generate<ToolsOnlySolution>();
    }

    private static void Builder_EventPreSolutionConfigure(Solution solution)
    {
        ConanDependencies.ParseConanFile(solution);
    }

    private static void Builder_EventPostProjectLink(Project project)
    {
        if(project is Application && !(project is ExternalProject) && !(project is ToolsProject))
        {
            Sharpmake.Resolver resolver = new Sharpmake.Resolver();
            resolver.SetParameter("project", project);
            string outdir = @"[project.SharpmakeCsPath]\build\reflection\dumps\[project.Name].txt";
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