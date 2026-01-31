using System.Text;
using Microsoft.EntityFrameworkCore;

namespace CSharpParser.model
{
    class CsharpDbContext : DbContext
    {
        public CsharpDbContext(DbContextOptions options) : base(options) { }
        
        public DbSet<CsharpAstNode> CsharpAstNodes { get; set; }
        public DbSet<CsharpNamespace> CsharpNamespaces { get; set; }
        public DbSet<CsharpClass> CsharpClasses { get; set; }
        public DbSet<CsharpMethod> CsharpMethods { get; set; }
        public DbSet<CsharpVariable> CsharpVariables { get; set; }
        public DbSet<CsharpStruct> CsharpStructs { get; set; }
        public DbSet<CsharpEnum> CsharpEnums { get; set; }
        public DbSet<CsharpEnumMember> CsharpEnumMembers { get; set; }        
        public DbSet<CsharpEtcEntity> CsharpEtcEntitys { get; set; }


    }

}
