using System.IO;
using System;
using System.Linq;
using Sharpmake;


public class Utils
{
    public static string g_FilterFolderEngine = "Engine";
    public static string g_FilterFolderLibraries = "Libraries";
    public static string g_FilterFolderGame = "Samples";
    public static string g_BuildFolder = "Build";
    public static string SourceFolderName = "Source";

    public static Target[] Targets
    {
        get
        {
            return new Target[]{
                new Target(
                  Platform.win64,
                  DevEnv.vs2022,
                  Optimization.Debug | Optimization.Release,
                  OutputType.Lib | OutputType.Dll)
            };
        }
    }


    public static void ConfigureProjectName(Project.Configuration conf, Target target)
    {
        conf.Name = @"[target.Optimization]_[target.OutputType]";

        conf.ProjectPath = @"[project.SharpmakeCsPath]\build\projects";
        conf.IntermediatePath = @"[project.SharpmakeCsPath]\build\intermediate\[target.Optimization]\[project.Name]_[target.DevEnv]_[target.Platform]";
        conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings();
        conf.VcxprojUserFile.LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]";
    }

    public static void ReferenceAllEngineLibraries(Project.Configuration conf, Target target)
    {
        conf.AddPrivateDependency<CoreModule>(target);
        conf.AddPrivateDependency<CliModule>(target);
        conf.AddPrivateDependency<EngineModule>(target);
        conf.AddPrivateDependency<ImGui>(target);
        conf.AddPrivateDependency<SDL2>(target);
        conf.AddPrivateDependency<OpTick>(target);
    }
}


public abstract class Module : Project
{
    public Module() : base()
    {
        FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();
        string rootDirectory = Path.Combine(fileInfo.DirectoryName, ".");
        RootPath = Util.SimplifyPath(rootDirectory);

        SourceRootPath = $"[project.SharpmakeCsPath]/{Utils.SourceFolderName}/[project.Name]";

        AddTargets(Utils.Targets);
    }

    [Configure(), ConfigurePriority(1)]
    virtual public void ConfigureAll(Configuration conf, Target target)
    {
        Utils.ConfigureProjectName(conf, target);
        conf.SolutionFolder = Utils.g_FilterFolderEngine;

        conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(Options.Vc.Compiler.MinimalRebuild.Enable);

        conf.Output = target.OutputType == OutputType.Lib ? Configuration.OutputType.Lib : Configuration.OutputType.Dll;

        if(conf.Output == Configuration.OutputType.Lib)
        {
            if(target.GetOptimization() == Optimization.Debug)
            {
                conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebug);
            }
            else
            {
                conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreaded);
            }
        }
        else if (conf.Output == Configuration.OutputType.Dll)
        {
            string capitalisedName = conf.Project.Name.ToUpper();
            conf.Defines.Add($"{capitalisedName}_DLL");
            conf.Defines.Add($"{capitalisedName}_EXPORTS");

            conf.ExportDefines.Add($"{capitalisedName}_DLL");

            if (target.GetOptimization() == Optimization.Debug)
            {
                conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDebugDLL);
            }
            else
            {
                conf.Options.Add(Options.Vc.Compiler.RuntimeLibrary.MultiThreadedDLL);
            }
        }


        conf.PrecompHeader = "[project.Name].pch.h";
        conf.PrecompSource = "[project.Name].pch.cpp";

        conf.Defines.Add("WIN32_LEAN_AND_MEAN");
        conf.Defines.Add("NOMINMAX");

        conf.Defines.Add("FEATURE_D2D");
        conf.Defines.Add("FEATURE_XAUDIO");

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP20);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);

        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189",  // Unused local variables
            "4251", // DLL export
            "4275" // DLL export
        ));

        conf.Options.Add(new Options.Vc.Linker.DisableSpecificWarnings(
            "4099" // No PDB with library.
        ));
        conf.Options.Add(Options.Vc.General.TreatWarningsAsErrors.Enable);

        conf.IncludePaths.Add(@"[project.SourceRootPath]");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/" + Utils.SourceFolderName);
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]");

        // Handle all conan packages
        conf.AddPublicDependency<ConanDependencies>(target);
    }

    [Configure(Blob.Blob)]
    public virtual void ConfigureBlob(Configuration conf, Target target)
    {
        conf.IsBlobbed = true;
        conf.IncludeBlobbedSourceFiles = false;
    }

    [Configure(Optimization.Debug), ConfigurePriority(2)]
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

public abstract class Application : Project
{
    public Application() : base()
    {
        FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();
        string rootDirectory = Path.Combine(fileInfo.DirectoryName, ".");
        RootPath = Util.SimplifyPath(rootDirectory);

        SourceRootPath = $"[project.SharpmakeCsPath]/{Utils.SourceFolderName}/[project.Name]";

        AddTargets(Utils.Targets);
    }

    [Configure(), ConfigurePriority(1)]
    virtual public void ConfigureAll(Configuration conf, Target target)
    {
        Utils.ConfigureProjectName(conf, target);

        conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(Options.Vc.Compiler.MinimalRebuild.Enable);

        conf.PrecompHeader = "[project.Name].pch.h";
        conf.PrecompSource = "[project.Name].pch.cpp";

        conf.Defines.Add("WIN32_LEAN_AND_MEAN");
        conf.Defines.Add("NOMINMAX");

        conf.Defines.Add("FEATURE_D2D");
        conf.Defines.Add("FEATURE_XAUDIO");

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP20);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189",  // Unused local variables
            "4251", // DLL export
            "4275" // DLL export

        ));

        conf.Options.Add(new Options.Vc.Linker.DisableSpecificWarnings(
            "4099" // No PDB with library.
        ));
        conf.Options.Add(Options.Vc.General.TreatWarningsAsErrors.Enable);

        conf.IncludePaths.Add(@"[project.SourceRootPath]");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/" + Utils.SourceFolderName);
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]");

        // Handle all conan packages
        conf.AddPublicDependency<ConanDependencies>(target);
        conf.AddPublicDependency<DirectXTK>(target);
    }

    [Configure(Blob.Blob)]
    public virtual void ConfigureBlob(Configuration conf, Target target)
    {
        conf.IsBlobbed = true;
        conf.IncludeBlobbedSourceFiles = false;
    }

    [Configure(Optimization.Debug), ConfigurePriority(2)]
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
        conf.SolutionFolder = "libraries";
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

public abstract class ExternalProject : Module
{
    public string ExternalDir = @"[project.SharpmakeCsPath]/external";

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        
        conf.Options.Add(Options.Vc.General.TreatWarningsAsErrors.Disable);

        conf.SolutionFolder = Utils.g_FilterFolderLibraries;

        conf.PrecompHeader = null;
        conf.PrecompSource = null;

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
        Executable = "";

        ExecutableArguments = string.Format("fxc /Zi /nologo /O2 /E\"{0}\" /T {1} /Fh\"{2}\" /Vn\"cso_{3}\" \"{4}\"", "main", profile.ToString(), Output, resourceName, filename);
    }


    public static void ConfigureShaderIncludes(Project.Configuration conf)
    {
        string outputDir = $"{conf.Project.RootPath}\\build\\shaders\\{conf.Target.GetOptimization()}\\{conf.Project.Name}";
        conf.IncludePaths.Add(outputDir);

    }

    public static void ClaimAllShaderFiles(Project project)
    {
        ClaimShaderFiles(project, ShaderProfile.vs_5_0, "_vx.hlsl", "main");
        ClaimShaderFiles(project, ShaderProfile.ps_5_0, "_px.hlsl", "main");
        ClaimShaderFiles(project, ShaderProfile.cs_5_0, "_cs.hlsl", "main");

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
                    string outputDir = string.Format(@"{0}\build\shaders\{2}\{1}\", project.SharpmakeCsPath, project.Name, conf.Target.GetOptimization());
                    CompileHLSL compileTask = new CompileHLSL(conf, profile, outputDir, targetName, Project.GetCapitalizedFile(file));
                    project.ResolvedSourceFiles.Add(compileTask.Output);
                    conf.CustomFileBuildSteps.Add(compileTask);
                }

            }

        }

    }
}
