use std::env;
use std::io;
use std::io::{BufReader,BufRead,Lines};
use std::fs;
use std::fs::File;
use std::collections::{HashMap, HashSet};
use std::path::Path;
use std::path::PathBuf;

fn read_lines(filename : &PathBuf)  -> io::Result<Lines<BufReader<File>>>
{
    let file = File::open(filename)?;
    Ok(BufReader::new(file).lines())
}

fn get_matching_in_dir(directory : &Path, extension: &Vec<&str>) -> Vec<PathBuf>
{
    let mut files = Vec::new();

    let dir  = fs::read_dir(directory);
    if let Ok(d) = dir {
        
        for entry in d {
            if let Ok(e) = entry {
                let p = &e.path();
                if e.file_type().unwrap().is_dir()
                {
                    let mut dir_files : Vec<_> = get_matching_in_dir(&p, &extension);
                    files.append(&mut dir_files);

                }
                else
                {
                    for ext in extension {
                        if p.extension().unwrap().to_str().unwrap() == *ext
                        {
                            files.push(p.into());
                        }
                    }
                }
            }
        }
    }


    files
}

#[derive(Debug)]
struct FileInfo
{
   // Top level headers this file is including
    headers : Vec<PathBuf> 
}

type IncludeGraph = std::collections::HashMap<PathBuf, FileInfo>;

fn traverse_graph<T>(graph: &IncludeGraph, file: &Path, visited : &mut Vec<PathBuf>, f : &T) -> bool where
    T: Fn(&FileInfo, &Path) -> bool
{
    if visited.contains(&file.to_path_buf()) {
        visited.push(file.to_path_buf());
        return true;
    }

    visited.push(file.to_path_buf());

    if let Some(info) = graph.get(&file.to_path_buf()) {
        f(&info, file);

        for header in &info.headers {
            let result = traverse_graph(graph, header, visited, f);
            if result {
                return true;
            }
        }
    }

    return false;
}

fn main() {
    print!("Args: ");
    for e in env::args() {
        print!("{e} ");
    }
    print!("\n");

    let args : Vec<String> = env::args().collect();
    let dirs : Vec<PathBuf>  = args.clone().into_iter().skip(1).filter(|v| !v.starts_with("-I")).map(|v| Path::new(&v).to_path_buf()).collect();
    let include_dirs : Vec<PathBuf>  = args.into_iter()
    .skip(1)
    .filter(|v| v.starts_with("-I"))
    .map(|value| {
        value[2..].into()
    })
    .collect();
    dbg!(&include_dirs);


    for d in &dirs {
        println!("Finding source files for {}", d.display());

        let extensions = vec![
            "h"
        ];
        let files : Vec<PathBuf> = get_matching_in_dir(d.as_path(), &extensions);
        let mut file_infos : HashMap<PathBuf, FileInfo> = HashMap::new();

        println!("Building file database.");
        for f in &files {

            // 1. Read the file
            if let Ok(file_contents) = read_lines(f) {

                // 2. Find any include statements
                let includes = 
                file_contents
                .filter(|f| {

                    let tmp = f.as_ref().unwrap();
                    tmp.starts_with("#include")  && !tmp.contains('<')
                })
                .map(|f| {

                    let line = f.unwrap();
                    let words = line.split(" ");
                    let w : String = words.skip(1).take(1).collect();
                    let w : String = w.replace("\"", "").replace("/", "\\").trim().into();
                    return w;
                });

                // 3. Resolve all the includes based on some simple include folder rules
                let resolved_includes = includes.filter_map(|include| {
                    let mut root = f.to_path_buf();
                    root.set_file_name(include.clone());

                    if root.exists() {
                        return Some(root);
                    }

                    // Try the include directories
                    let first_include = include_dirs.iter().map(|d| {
                        let p = Path::new(&d);
                        let p = p.join(include.clone()); 
                        return p;
                    }).find(|p| p.exists());

                    if let Some(value) = first_include {
                        return Some(value);
                    }

                    return None;
                });

                // #TODO: Include paths need to be resolved to full path
                file_infos.insert(f.clone(), FileInfo { headers: resolved_includes.collect() });

                // 3. For each include statement find matching file to include (recursive)
                //  3.1 First check in current directory (ignore system level includes '<>')
                //  3.2 Then check in root of all directories requested
                //  3.3 Still no match found -> Log as error 
            }
        }


        println!("Detecting cyclic includes.");
        for file in &files {


            let mut visited = Vec::new();
            let mut cyclic = traverse_graph(&file_infos, file, &mut visited, &|info, parent| {
                return true;
            });

            cyclic = cyclic && visited[visited.len() - 1] == visited[0];
            if cyclic {
                
                println!("Cyclic include found:");
                print!("(ROOT)\t{}\n", file.display());

                for v in visited.iter().skip(1) {
                    print!("\t\t -> {}\n", v.display());
                }
                print!("\n");
            }
        }
    }
}
