BEGIN{
    print "This script(network.awk) used to parser the network configure file and setup it."
    print "Copyright (C) 2011 GuoWenxue<guowenxue@gmail.com>"
}

{
  if( NR>start_line && NR<end_line)
  {
    split ($0, x, "=");

    #MAC,IP,Netmask settings
    if(x[1] ~ /status/)
    {
       status=x[2]          
    }

    if(x[1] ~ /macaddr/)
    {
       macaddr=x[2]          
    }

    if(x[1] ~ /ipaddr0/)
    {
       ipaddr0=x[2]          
    }

    if(x[1] ~ /netmask0/)
    {
       netmask0=x[2]          
    }

    if(x[1] ~ /ipaddr1/)
    {
       ipaddr1=x[2]          
    }

    if(x[1] ~ /netmask1/)
    {
       netmask1=x[2]          
    }

    #DHCP settings
    if(x[1] ~ /dhcp.status/)
    {
      dhcp_status=x[2]          
    }

    if(x[1] ~ /dhcp.start/)
    {
       dhcp_start=x[2]          
    }

    if(x[1] ~ /dhcp.end/)
    {
      dhcp_end=x[2]          
    }

    if(x[1] ~ /dhcp.netmask/)
    {
       dhcp_netmask=x[2]          
    }

    if(x[1] ~ /dhcp.gateway/)
    {
       dhcp_gateway=x[2]          
    }

    if(x[1] ~ /dhcp.dns1/)
    {
       dhcp_dns1=x[2]          
    }

    if(x[1] ~ /dhcp.dns2/)
    {
       dhcp_dns2=x[2]          
    }

    if(x[1] ~ /dhcp.lease/)
    {
       dhcp_lease=x[2]          
    }
  }
}

END{
    if ("on" == status)
    {
       if ("" != macaddr)
       {
           cmd ="ifconfig " ifname " down";
           #print cmd;
           system(cmd)
           
           cmd="ifconfig " ifname " hw ether " macaddr " up"
           #print cmd
           system(cmd)
       }

       if("" != ipaddr0 && "" != netmask0)
       {
           cmd="ifconfig " ifname " " ipaddr0 " netmask " netmask0 " up"
           #print cmd;
           system(cmd);
       }

       if("" != ipaddr1 && "" != netmask1)
       {
           cmd="ifconfig " ifname ":1 " ipaddr1 " netmask " netmask1 " up"
           #print cmd;
           system(cmd);
       }
    }

    if ("on" == dhcp_status && "" != dhcp_start && "" != dhcp_end && "" != dhcp_netmask)
    {
         dhcp_cfg="interface eth0\n";
         dhcp_cfg=dhcp_cfg "start " dhcp_start "\n";
         dhcp_cfg=dhcp_cfg "end " dhcp_end "\n";
         dhcp_cfg=dhcp_cfg "option subnet " dhcp_netmask "\n";
         if ("" != dhcp_gateway)
            dhcp_cfg=dhcp_cfg "opt router " dhcp_gateway "\n";
         if ("" != dhcp_dns1)
            dhcp_cfg=dhcp_cfg "option dns " dhcp_dns1 "\n";
         if ("" != dhcp_dns2)
            dhcp_cfg=dhcp_cfg "option dns " dhcp_dns2 "\n";
         if ("" != dhcp_lease)
            dhcp_cfg=dhcp_cfg "option lease " dhcp_lease "\n";

         cfg_file="dhcp_"ifname"_cfg"

         #Generate DHCP configure file
         system ("echo \"" dhcp_cfg "\" > /tmp/"cfg_file);

         # Start DHCP server
         system ("udhcpd " cfg_file);
    }

#    print "==========================================="
}
