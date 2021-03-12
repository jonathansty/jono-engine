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
public abstract class JonaBaseProject : Project
{
    public JonaBaseProject() : base()
    {
        FileInfo fileInfo = Util.GetCurrentSharpmakeFileInfo();
        string rootDirectory = Path.Combine(fileInfo.DirectoryName, ".");
        RootPath = Util.SimplifyPath(rootDirectory);

        SourceRootPath = @"[project.SharpmakeCsPath]\src\[project.Name]";

        AddTargets(Utils.Targets);
    }

    [Configure(), ConfigurePriority(1)]
    virtual public void ConfigureAll(Configuration conf, Target target)
    {
        Utils.ConfigureProjectName(conf, target);

        conf.Options.Add(Options.Vc.General.WindowsTargetPlatformVersion.Latest);
        // conf.Options.Add(Options.Vc.General.PlatformToolset.ClangCL);
        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(Options.Vc.Compiler.MinimalRebuild.Enable);

        // conf.AdditionalCompilerOptions.Add("-Wno-unused-parameter");
        // conf.AdditionalCompilerOptions.Add("-Wno-reorder-ctor");
        // conf.AdditionalCompilerOptions.Add("-Wno-unused-variable");
        // conf.AdditionalCompilerOptions.Add("-Wno-unused-private-field");
        // conf.AdditionalCompilerOptions.Add("-Wno-format-security");

        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP17);
        conf.Options.Add(Options.Vc.General.CharacterSet.Unicode);

        conf.Options.Add(Options.Vc.Compiler.Exceptions.EnableWithSEH);
        conf.Options.Add(new Options.Vc.Compiler.DisableSpecificWarnings(
            "4100", // Unused method variables
            "4189"  // Unused local variables
        ));

        conf.IncludePaths.Add(@"[project.SourceRootPath]");

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

public abstract class ExternalProject : JonaBaseProject
{
    public string ExternalDir = @"[project.SharpmakeCsPath]/external";

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "libraries";
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
        string outputDir = $"{conf.Project.RootPath}/obj/{conf.Project.Name}_{conf.Target.GetOptimization()}";
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
                    string outputDir = string.Format(@"{0}\obj\{1}_{2}\shaders\", project.SharpmakeCsPath, project.Name, conf.Target.GetOptimization());
                    CompileHLSL compileTask = new CompileHLSL(conf, profile, outputDir, targetName, Project.GetCapitalizedFile(file));
                    project.ResolvedSourceFiles.Add(compileTask.Output);
                    conf.CustomFileBuildSteps.Add(compileTask);
                }

            }

        }

    }
}
