using Sharpmake;
using System.IO;

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
public class RTTR : ExternalProject
{

    public RTTR() : base()
    {
        Name = "RTTR";
        SourceRootPath = Path.Combine(ExternalDir, "rttr/src/rttr");
        //SourceFilesFiltersRegex = new Strings();
        //SourceFilesFiltersRegex.Add(@"rttr\\\w*\.(h|cpp)");
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(@"[project.SourceRootPath]/..");

        conf.Output = Configuration.OutputType.Lib;

    }
    public override void ConfigureRelease(Configuration conf, Target target)
    {
        base.ConfigureRelease(conf, target);
        //conf.ExportDefines.Add(@"RTTR_DLL=[project.VcpkgDir]/bin;");
        conf.LibraryFiles.Add(@"rttr_core");
    }

    public override void ConfigureDebug(Configuration conf, Target target)
    {
        base.ConfigureDebug(conf, target);
        //conf.ExportDefines.Add(@"RTTR_DLL=[project.VcpkgDir]/debug/bin;");
        conf.LibraryFiles.Add(@"rttr_core_d");
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

        // FreeType is a dependency
        conf.AddPrivateDependency<FreeType>(target, DependencySetting.DefaultWithoutLinking);

        conf.Output = Configuration.OutputType.Lib;
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]");
        conf.IncludeSystemPaths.Add(@"[project.SourceRootPath]/examples");
    }
}

[Export]
public class HLSLPP : ExternalProject
{
    public HLSLPP() : base()
    {
        Name = "hlslpp";
        SourceRootPath = Path.Combine(ExternalDir, "hlslpp/");
        NatvisFiles.Add(Path.Combine(SourceRootPath, "include/hlsl++.natvis"));
    }

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludeSystemPaths.Add(Path.Combine(ExternalDir, "[project.Name]/include"));
        conf.ExportDefines.Add("HLSLPP_FEATURE_TRANSFORM");
    }


}


[Generate]
public class EnkiTS : ExternalProject
{
    public EnkiTS()
    {
        Name = "EnkiTS";
        SourceRootPath = Path.Combine(ExternalDir, "enkiTS/");
        SourceFilesExcludeRegex.Add("example");

    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(Path.Combine(ExternalDir, "enkiTS/src"));
        conf.Output = Configuration.OutputType.Lib;
    }
}

[Export]
public class LibClang : VCPKG
{
    public LibClang()
    {
        Name = "LibClang";

    }

    public override void ConfigureRelease(Configuration config, Target target)
    {
        base.ConfigureRelease(config, target);
        config.LibraryFiles.Add("libclang");
    }
    public override void ConfigureDebug(Configuration config, Target target)
    {
        base.ConfigureDebug(config, target);
        config.LibraryFiles.Add("libclangd");
    }

}

[Export]
public class ClReflect : ExternalProject
{
    public ClReflect()
    {
        Name = "clReflect";
        SourceRootPath = @"[project.ExternalDir]/clReflect";
    }

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludeSystemPaths.Add(@"[project.ExternalDir]/[project.Name]/inc");
    }
}

[Export]
public class Fmt : VCPKG
{
    public Fmt() : base(false)
    {
    }

    public override void ConfigureDebug(Configuration conf, Target target)
    {
        base.ConfigureDebug(conf, target);
        conf.LibraryFiles.Add(@"fmtd");
    }

    public override void ConfigureRelease(Configuration conf, Target target)
    {
        base.ConfigureRelease(conf, target);
        conf.LibraryFiles.Add(@"fmt");
    }


}
