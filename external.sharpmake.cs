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
        SourceRootPath = Path.Combine(externalDir, "hlslpp/");
        NatvisFiles.Add(Path.Combine(SourceRootPath, "include/hlsl++.natvis"));
    }

    override public void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.Output = Configuration.OutputType.None;
        conf.IncludeSystemPaths.Add(Path.Combine(externalDir, "[project.Name]/include"));
        conf.ExportDefines.Add("HLSLPP_FEATURE_TRANSFORM");
    }


}


[Generate]
public class EnkiTS : ExternalProject
{
    public EnkiTS()
    {
        Name = "EnkiTS";
        SourceRootPath = Path.Combine(externalDir, "enkiTS/");

    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(Path.Combine(externalDir, "enkiTS/src"));
        conf.Output = Configuration.OutputType.Lib;
    }
}

[Export]
public class Refl : ExternalProject
{
    public Refl()
    {
        Name = "refl-cpp";
        SourceRootPath = Path.Combine(externalDir, "refl-cpp/");

    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(Path.Combine(externalDir, "refl-cpp/"));
    }
}