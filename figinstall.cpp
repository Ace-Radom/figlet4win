#include<codecvt>
#include<iostream>
#include<iomanip>
#include<cassert>
#include<filesystem>
#include<fstream>
#include<string>
#include<sstream>
#include<unordered_map>
#include<vector>
#include<getopt.h>

#include"curl/curl.h"
#include"openssl/sha.h"

#include<windows.h>

#ifdef INFO
#undef INFO
#endif

#ifdef ERROR
#undef ERROR
#endif

#ifdef FAILED
#undef FAILED
#endif

#define OK          0
#define FAILED      1

#define FLF_T       0
#define FLC_T       1
#define TLF_T       2

const std::unordered_map<short,std::pair<std::string,std::string>> FIGFONT_ITEM_TYPE_NAME_MAP = {
    { FLF_T , { "FIGfont"             , ".flf" } } ,
    { FLC_T , { "FIGlet Control File" , ".flc" } } ,
    { TLF_T , { "TOIlet Font"         , ".tlf" } }
};

#ifndef NDEBUG

#define INFO( msg )                                                                             \
    std::cout << __FUNCTION__ << ": " << msg << std::endl;

#else

#define INFO( msg )                                                                             \
    std::cout << msg << std::endl;

#endif

#define ERROR( msg )                                                                            \
    std::cerr << __FUNCTION__ << ": " << msg << std::endl;

#define CLEANUP_SIGN cleanup
#define CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , curlcode , curloption , data )               \
    curlcode = curl_easy_setopt( curl , curloption , data );                                    \
    if ( curlcode != CURLE_OK )                                                                 \
    {                                                                                           \
        ERROR( "Failed to set " << #curloption << ": " << curl_easy_strerror( curlcode ) );     \
        goto CLEANUP_SIGN;                                                                      \
    }

typedef struct {
    std::string shorten;
    std::string full;
    short type;
    std::string hash; // sha256 hash
} figfont_node_t;
typedef std::vector<figfont_node_t> figfont_list_t;

bool restrictive_mode = false;
bool opt_list = false;
bool remote_font = false;
std::string opt_list_v;

SHA256_CTX* calibration_sha;

int curl_write_string_callback( char* data , size_t size , size_t nmemb , void* userdata ){
    std::string* content = ( std::string* ) userdata;
    size_t realsize = size * nmemb;
    content -> append( data );
    return realsize;
}

int curl_write_file_callback( char* data , size_t size , size_t nmemb , void* userdata ){
    std::ofstream* wFile = ( std::ofstream* ) userdata;
    size_t realsize = size * nmemb;
    wFile -> write( data , realsize );
    SHA256_Update( calibration_sha , data , realsize );
    return realsize;
}

std::string get_figfont_base_meta(){
    CURL* curl = nullptr;
    CURLcode res;
    std::string content;
    
    curl = curl_easy_init();
    if ( !curl )
    {
        ERROR( "Init curl function failed" );
        res = CURLE_GOT_NOTHING;
        // no specific meaning: only a sign
        goto CLEANUP_SIGN;
    }

    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_HTTPGET , 1L );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_URL , "https://raw.githubusercontent.com/Ace-Radom/FIGfont_base/main/META" );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_WRITEFUNCTION , curl_write_string_callback );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_WRITEDATA , &content );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_SSL_VERIFYPEER , false );
    res = curl_easy_perform( curl );
    if ( res != CURLE_OK )
    {
        ERROR( "Failed to connect to GitHub gateway: " << curl_easy_strerror( res ) );
        goto CLEANUP_SIGN;
    }

    long rc;
    curl_easy_getinfo( curl , CURLINFO_RESPONSE_CODE , &rc );
    if ( rc != 200 )
    {
        ERROR( "Failed to get FIGfont base metadata: " << rc );
        content.clear();
        // do not return any msg when response failed
        res = CURLE_GOT_NOTHING;
    }

CLEANUP_SIGN:
    if ( curl )
    {
        curl_easy_cleanup( curl );
    }

    return content;
}

figfont_list_t parse_figfont_base_meta( std::string meta ){
    assert( !meta.empty() );

    std::istringstream iss( meta );
    std::string line;
    short type;
    figfont_list_t res;
    while ( std::getline( iss , line ) )
    {
        if ( line == "#FIGfonts" )
        {
            type = FLF_T;
        }
        else if ( line == "#control_files" )
        {
            type = FLC_T;
        }
        else if ( line == "#tlf" )
        {
            type = TLF_T;
        }
        else
        {
            std::string shorten = line.substr( 0 , line.find( ':' ) );
            std::string full = line.substr( line.find( ':' ) + 1 , line.find( '|' ) - line.find( ':' ) - 1 );
            std::string hash = line.substr( line.find( '|' ) + 1 );
            assert( !shorten.empty() && !full.empty() );
            res.push_back( { shorten , full , type , hash } );
        }
    }
    return res;
}

int list_remote_fonts( std::string prefix ){
    std::string meta = get_figfont_base_meta();
    if ( meta.empty() )
    {
        ERROR( "Failed to get FIGfont base metadata" );
        return FAILED;
    }
    figfont_list_t font_list = parse_figfont_base_meta( meta );
    assert( !font_list.empty() );
    std::cout << "Font base metadata got, " << font_list.size() << " items in total" << std::endl;
    if ( !prefix.empty() )
    {
        std::cout << "Prefix \"" << prefix << "\" matches:" << std::endl;
    }

    std::cout << FIGFONT_ITEM_TYPE_NAME_MAP.at( FLF_T ).first << std::endl;
    int i = 0;
    for ( ; i < font_list.size() && font_list[i].type == FLF_T ; i++ )
    {
        if ( !prefix.empty() && font_list[i].shorten.find( prefix ) == std::string::npos && font_list[i].full.find( prefix ) == std::string::npos )
        {
            continue;
        }
        std::cout << " * " << font_list[i].shorten << ": " << font_list[i].full << std::endl;
    }
    std::cout << FIGFONT_ITEM_TYPE_NAME_MAP.at( FLC_T ).first << std::endl;
    for ( ; i < font_list.size() && font_list[i].type == FLC_T ; i++ )
    {
        if ( !prefix.empty() && font_list[i].shorten.find( prefix ) == std::string::npos && font_list[i].full.find( prefix ) == std::string::npos )
        {
            continue;
        }
        std::cout << " * " << font_list[i].shorten << ": " << font_list[i].full << std::endl;
    }
    std::cout << FIGFONT_ITEM_TYPE_NAME_MAP.at( TLF_T ).first << std::endl;
    for ( ; i < font_list.size() && font_list[i].type == TLF_T ; i++ )
    {
        if ( !prefix.empty() && font_list[i].shorten.find( prefix ) == std::string::npos && font_list[i].full.find( prefix ) == std::string::npos )
        {
            continue;
        }
        std::cout << " * " << font_list[i].shorten << ": " << font_list[i].full << std::endl;
    }
    return OK;
}

std::filesystem::path download_remote_font_to_temp( const figfont_node_t& item ){
    std::string url;
    std::ostringstream oss;
    oss << "https://raw.githubusercontent.com/Ace-Radom/FIGfont_base/main/" << item.full << FIGFONT_ITEM_TYPE_NAME_MAP.at( item.type ).second;
    url = oss.str();

    CURL* curl = nullptr;
    CURLcode res;
    std::ofstream wFile;

    unsigned char hash[SHA256_DIGEST_LENGTH];
    std::string sha256;

    TCHAR temp_dir[MAX_PATH];
    if ( !GetTempPath( MAX_PATH , temp_dir ) )
    {
        ERROR( "Failed to get system temp path" );
        return std::filesystem::path();
    } // curl hasn't been inited yet
    std::filesystem::path temp_file_path( temp_dir );
    temp_file_path = temp_file_path / "FIGlet4Win" / "fonts" / ( item.full.substr( 0 , item.full.find( '/' ) ) );
    if ( !std::filesystem::exists( temp_file_path ) )
    {
        std::filesystem::create_directories( temp_file_path );
    }
    temp_file_path /= ( item.full.substr( item.full.find( '/' ) + 1 ) + FIGFONT_ITEM_TYPE_NAME_MAP.at( item.type ).second );
    wFile.open( temp_file_path , std::ios::out | std::ios::binary );
    if ( !wFile.is_open() )
    {
        ERROR( "Cannot write download temp file: " << strerror( errno ) );
        return std::filesystem::path();
    }

    calibration_sha = new SHA256_CTX;
    SHA256_Init( calibration_sha );

    curl = curl_easy_init();
    if ( !curl )
    {
        ERROR( "Init curl function failed" );
        res = CURLE_GOT_NOTHING;
        goto CLEANUP_SIGN;
    }

    INFO( "Downloading " << item.full << " from " << url );

    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_HTTPGET , 1L );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_URL , url.c_str() );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_WRITEFUNCTION , curl_write_file_callback );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_WRITEDATA , &wFile );
    CURL_SETOPT_IF_FAILED_GOTO_CLEANUP( curl , res , CURLOPT_SSL_VERIFYPEER , false );
    res = curl_easy_perform( curl );
    if ( res != CURLE_OK )
    {
        ERROR( "Failed to connect to GitHub gateway: " << curl_easy_strerror( res ) );
        goto CLEANUP_SIGN;
    }

    long rc;
    curl_easy_getinfo( curl , CURLINFO_RESPONSE_CODE , &rc );
    if ( rc != 200 )
    {
        ERROR( "Failed to download temp font file: " << rc );
        temp_file_path.clear();
        // do not return the temp file path when response failed
        res = CURLE_GOT_NOTHING;
        goto CLEANUP_SIGN;
    }

    memset( hash , 0 , sizeof( hash ) );
    SHA256_Final( hash , calibration_sha );
    for ( int i = 0 ; i < SHA256_DIGEST_LENGTH ; i++ )
    {
        char dump_hex_buf[3];
        memset( dump_hex_buf , 0 , sizeof( dump_hex_buf ) );
        sprintf( dump_hex_buf , "%02x" , hash[i] );
        sha256.append( dump_hex_buf );
    }
    if ( sha256 != item.hash )
    {
        ERROR( "Failed to validate download content" );
        temp_file_path.clear();
        res = CURLE_GOT_NOTHING;
        goto CLEANUP_SIGN;
    }

    INFO( "Download content validated" );

CLEANUP_SIGN:
    if ( curl )
    {
        curl_easy_cleanup( curl );
    }
    delete calibration_sha;
    wFile.close();
    return temp_file_path;
}

void print_helpmsg(){
    std::cout << "Usage: figinstall [ -?Ih ] | [ [ -L | -l ] | [ -rR ] ] [ font ]" << std::endl;
    return;
}

void print_infomsg(){
    std::cout << "FIGinstall, FIGlet Font Installer" << std::endl
              << "Copyright (C) 2024 Sichen Lyu" << std::endl
              << "Version: " << FIGLET4WINVERSION << ", date: " << FIGLET4WINBUILDTIME << std::endl
              << "FIGlet4Win, along with the various FIGlet fonts and documentation, may be" << std::endl
              << "freely copied and distributed." << std::endl
              << "Contributes, bug reports and feature requests to FIGlet4Win are welcome." << std::endl
              << "Repo-site: <https://github.com/Ace-Radom/figlet4win>" << std::endl;
    return;
}

int main( int argc , char** argv ){
    SetConsoleCP( CP_UTF8 );
    SetConsoleOutputCP( CP_UTF8 );

    if ( argc == 1 )
    {
        print_helpmsg();
        exit( OK );
    }

    int c;
    while ( ( c = getopt( argc , argv , "?IhrLl:R" ) ) != -1 )
    {
        switch ( c ) {
            case '?': print_helpmsg(); exit( OK ); break;
            case 'I': print_infomsg(); exit( OK ); break;
            case 'h': print_helpmsg(); exit( OK ); break;
            case 'r': restrictive_mode = true; break;
            case 'L': opt_list = true; break;
            case 'l': opt_list = true; opt_list_v = optarg; break;
            case 'R': remote_font = true; break;
            default: print_helpmsg(); exit( FAILED ); break;
        }
    }

    curl_global_init( CURL_GLOBAL_ALL );

    int rc = OK;
    int res;
    std::filesystem::path install_target;
    std::filesystem::path chkfont_path;
    TCHAR figinstall_path[MAX_PATH];
    std::string chkfont_cmd;
    std::wstring chkfont_cmd_w;
    int conv_req_size;  // string <-> wstring convert required size
    std::filesystem::path install_to_path;
    std::ifstream rFile;
    std::ofstream wFile;
    char copy_buf[1024];

    if ( opt_list )
    {
        rc = list_remote_fonts( opt_list_v );
        goto curl_global_cleanup;
    }

    if ( optind < argc - 1 )
    {
        ERROR( "Installing multiple font file is not supported" );
        rc = FAILED;
        goto curl_global_cleanup;
    } // this may be a feature in the future

    if ( remote_font )
    {
        std::string target = argv[argc - 1];
        std::string meta = get_figfont_base_meta();
        if ( meta.empty() )
        {
            ERROR( "Failed to get FIGfont base metadata" );
            rc = FAILED;
            goto curl_global_cleanup;
        }
        INFO( "Got metadata from remote" )
        figfont_list_t font_list = parse_figfont_base_meta( meta );
        figfont_list_t match_list;
        for ( const auto& it : font_list )
        {
            if ( it.shorten == target || it.full == target )
            {
                match_list.push_back( it );
            }
        } // find all matched items
        if ( match_list.empty() )
        {
            ERROR( "Target \"" << target << "\" cannot match anything" );
            rc = FAILED;
            goto curl_global_cleanup;
        } // empty match list
        if ( match_list.size() > 1 )
        {
            std::cout << "Multiple matched items found:" << std::endl;
            for ( int i = 0 ; i < match_list.size() ; i++ )
            {
                std::cout << i + 1 << ". " << std::left << std::setw( 24 ) << FIGFONT_ITEM_TYPE_NAME_MAP.at( match_list[i].type ).first << match_list[i].shorten << ":" << match_list[i].full << std::endl;
            }
            while ( 1 )
            {
                std::cout << "Please choose one item to install: ";
                int in;
                std::cin >> in;
                if ( in >= 1 && in <= match_list.size() )
                {
                    install_target = download_remote_font_to_temp( match_list[in - 1] );
                    if ( match_list[in - 1].type != FLF_T )
                    {
                        restrictive_mode = false;
                    }
                }
            }
        } // more than one match results
        else
        {
            install_target = download_remote_font_to_temp( match_list[0] );
            if ( match_list[0].type != FLF_T )
            {
                restrictive_mode = false;
            }
        } // one result only
    } // remote font
    else
    {
        install_target = argv[argc - 1];
    } // local font

    if ( !GetModuleFileName( NULL , figinstall_path , MAX_PATH ) )
    {
        ERROR( "Failed to get FIGlet binary path" );
        rc = FAILED;
        goto curl_global_cleanup;
    }

    if ( restrictive_mode )
    {
        chkfont_path = figinstall_path;
        chkfont_path = chkfont_path.parent_path() / "chkfont.exe";
        chkfont_cmd = std::string( "\"" ) + chkfont_path.string() + "\" " + install_target.string();
        // `"xxx\chkfont.exe" xxx.flf`
        chkfont_cmd_w = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes( chkfont_cmd );
        conv_req_size = WideCharToMultiByte(
            CP_ACP ,
            0 ,
            chkfont_cmd_w.c_str() ,
            -1 ,
            NULL ,
            0 ,
            NULL ,
            NULL
        );
        chkfont_cmd.clear();
        WideCharToMultiByte(
            CP_ACP ,
            0 ,
            chkfont_cmd_w.c_str() ,
            -1 ,
            &chkfont_cmd[0] ,
            conv_req_size ,
            NULL ,
            NULL
        );
        // convert command to ANSI charcode
        res = system( chkfont_cmd.c_str() );
        // call chkfont
        if ( res != 0 )
        {
            ERROR( "Unexcepted format error occured" );
            rc = FAILED;
            goto curl_global_cleanup;
        }
    } // restrictive mode, call chkfont
    
    install_to_path = figinstall_path;
    install_to_path = install_to_path.parent_path() / DEFAULTFONTDIR / install_target.filename();

    if ( std::filesystem::exists( install_to_path ) )
    {
        char c;
        while ( 1 )
        {
            std::cout << "This item already exists, overwrite? (y/n): ";
            std::cin >> c;
            if ( c == 'y' || c == 'Y' )
            {
                break;
            }
            else if ( c == 'n' || c == 'N' )
            {
                INFO( "Exiting" );
                rc = OK;
                goto curl_global_cleanup;
            }
        }
    }

    rFile.open( install_target , std::ios::in | std::ios::binary );
    if ( !rFile.is_open() )
    {
        INFO( install_target );
        ERROR( "Failed to read install target item: " << strerror( errno ) );
        rc = FAILED;
        goto curl_global_cleanup;
    }
    wFile.open( install_to_path , std::ios::out | std::ios::binary );
    if ( !wFile.is_open() )
    {
        ERROR( "Failed to write install target item: " << strerror( errno ) );
        rc = FAILED;
        goto curl_global_cleanup;
    }
    while ( !rFile.eof() )
    {
        rFile.read( copy_buf , 1024 );
        std::streamsize bRead = rFile.gcount();
        wFile.write( copy_buf , bRead );
    }
    rFile.close();
    wFile.close();
    // fs::copy_file contains a bug in MinGW
    // it cannot overwrite existing targets even if fs::copy_options::overwrite_existing is set

    INFO( "Work dowe and success" );

curl_global_cleanup:
    curl_global_cleanup();

    return rc;
}