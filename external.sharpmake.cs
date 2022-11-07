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

        conf.AddPublicDependency<SDL2>(target);

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludePaths.Add(Path.Combine(ExternalDir, "SDL2/SDL2-2.24.1/include/SDL2"));
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]");
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]/examples");
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

[Generate]
public class DirectXTK : ExternalProject
{
    public DirectXTK() : base()
    {
        Name = "DirectXTK";
        SourceRootPath = @"[project.ExternalDir]/DirectXTK/";

        SourceFilesExtensions.Add("inc");
        SourceFilesExcludeRegex.Add(".*(XBOX|Model(.h|.cpp)).*");
        SourceFilesFiltersRegex.Add(".*(Src)|(Inc).*");
    }
    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);
        //conf.ProjectReferencesByPath.Add(@"[project.SharpmakeCsPath]/external/DirectXTK/DirectXTK_Desktop_2022_Win10.vcxproj");
        conf.PrecompHeader = "pch.h";
        conf.PrecompSource = "[project.ExternalDir]/DirectXTK/Src/pch.cpp";

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludePaths.Add(@"[project.SourceRootPath]/Inc");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/Src");
        conf.IncludePaths.Add(@"[project.SourceRootPath]/Src/Shaders/Compiled/");

        conf.IncludeSystemPaths.Add(Path.Combine(ExternalDir, "DirectXTK/Inc/"));
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
