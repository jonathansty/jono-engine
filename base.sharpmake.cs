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

public class ExternalProject : JonaBaseProject
{
    protected string externalDir = @"[project.SharpmakeCsPath]/external";

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
