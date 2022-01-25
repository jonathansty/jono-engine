using Sharpmake;
using System.IO;
using System.Collections;
using System.Collections.Generic;

[Export]
public class ConanDependencies : Sharpmake.Project
{

    struct Key
    {
        public Key(string t, Optimization opt)
        {
            type = t;
            optimization = opt;
        }

        public string type;
        public Optimization optimization;
    }

    private static System.Threading.Mutex globalMutex = new System.Threading.Mutex();
    private static Dictionary<Key, Strings> conanInformation = new Dictionary<Key, Strings>();
    public static void ParseConanFile(Solution solution)
    {
        lock (globalMutex)
        {
            foreach (var target in solution.Targets.TargetObjects)
            {
                string p = "Debug";
                switch (target.GetOptimization())
                {
                    case Optimization.Debug:
                        p = "Debug";
                        break;
                    case Optimization.Release:
                        p = "Release";
                        break;
                }

                string dir = Path.Combine("[solution.SharpmakeCsPath]\\build\\", "conan_"+p, "conanbuildinfo.txt");

                Sharpmake.Resolver resolver = new Sharpmake.Resolver();
                resolver.SetParameter("solution", solution);
                string outdir = resolver.Resolve(dir);

                string mode = "";
                foreach (var line in File.ReadLines(outdir))
                {
                    if (line.Length == 0)
                        continue;


                    if (line[0] == '[')
                    {
                        mode = line.Substring(1, line.Length - 2);
                    }
                    else
                    {
                        Key k = new Key();
                        k.type = mode;
                        k.optimization = target.GetOptimization();

                        if (!conanInformation.ContainsKey(k))
                        {
                            conanInformation.Add(k, new Strings());
                        }
                        conanInformation[k].Add(line);
                    }
                }


            }

        }
    }

    public ConanDependencies()
    {
        Name = "ConanDependencies";
        AddTargets(Utils.Targets);

    }

    [Configure(Optimization.Debug)]
    public void ConfigureDebug(Configuration conf, Target target)
    {
        conf.IncludePaths.AddRange(conanInformation[new Key("includedirs", Optimization.Debug)]);
        conf.LibraryPaths.AddRange(conanInformation[new Key("libdirs", Optimization.Debug)]);
        conf.LibraryFiles.AddRange(conanInformation[new Key("libs", Optimization.Debug)]);
    }

    [Configure(Optimization.Release)]
    public void ConfigureRelease(Configuration conf, Target target)
    {
        conf.IncludePaths.AddRange(conanInformation[new Key("includedirs", Optimization.Release)]);
        conf.LibraryPaths.AddRange(conanInformation[new Key("libdirs", Optimization.Release)]);
        conf.LibraryFiles.AddRange(conanInformation[new Key("libs", Optimization.Release)]);
    }
}

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

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]");
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]/examples");
    }
}

[Generate]
public class ImGuizmo : ExternalProject
{
    public ImGuizmo() : base()
    {
        Name = "ImGuizmo";
        SourceRootPath = Path.Combine(ExternalDir, "ImGuizmo/");
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

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "rttr/src"));
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "rttr/build/src"));
    }
}



