
var plaqueini = new Array();
var plaque = new Array();
var resultat = 0;
var meilleurecart = 2000000000;
var meilleurlevel = 2000000000;

var steps = '';
var res = getResult(new Array(2,3,4,7,10,100), 851, steps);

alert(steps);

function getResult(numbers, target, steps)
{
    //var plaqueini, resultat, plaqueini, plaque, meilleurecart,
    //    meilleurlevel, bestsolution;
    for(i=0; i<6; i++)
    {
        plaqueini[i] = numbers[i];
        if (numbers[i] == target)
        {
            steps = "target = target";
            return target;
        }
    }
    resultat = target;

    for (i=0;i<16;i++)
        for (j=0;j<16;j++)
            plaque[i][j] = 0;

    for(i=0; i<6; i++)
        plaque[6][i] = plaqueini[i];

    steps = compte(6);
    if (steps != '')
        return target;

    steps = affichesolution(meilleurlevel);
    return bestsolution['resultat'][meilleurlevel+1];
}

function affichesolution(l)
{
    //global bestsolution;
    res = '';
    for(i=6; i>l; i--)
        res +=
            bestsolution['valeur1'][i] + ' ' +
            bestsolution['operation'][i] + ' ' +
            bestsolution['valeur2'][i] + ' = ' +
            bestsolution['resultat'][i] + "\n";
    return res;
}

function compte(l)
{
    //global resultat, plaque, meilleurecart,
    //    bestsolution, savesolution, meilleurlevel;

    ecart = resultat - plaque[l][0];
    if (ecart < 0)
        ecart =- ecart;
    if (ecart <= meilleurecart)
    {
        if(ecart<meilleurecart)
        {
            meilleurecart=ecart;
            bestsolution=savesolution;
            meilleurlevel=l;
            if(!ecart)
                return affichesolution(l);
        }
        else  // (ecart==meilleurecart)
        {
            if(l>meilleurlevel)
            {
                bestsolution=savesolution;
                meilleurlevel=l;
                if(!ecart)
                    return affichesolution(l);
            }
        }
    }

    if(l==1)
        return;

    for(i=0;i<l-1;i++)
    {
        for(j=i+1;j<l;j++)
        {
            plaque1=plaque[l][i];
            plaque2=plaque[l][j];
            n=1;
            for(k=0;k<l;k++)
            {
                if(k!=i && k!=j)
                {
                    plaque[l-1][n]=plaque[l][k];
                    n++;
                }
            }
            plaque[l-1][0]=plaque1+plaque2;
            savesolution['valeur1'][l]=plaque1;
            savesolution['operation'][l]='+';
            savesolution['valeur2'][l]=plaque2;
            savesolution['resultat'][l]=plaque[l-1][0];
            s = compte(l-1);
            if (s != '')
                return s;
            if(plaque1!=1 && plaque2!=1)
            {
                plaque[l-1][0]=plaque1*plaque2;
                savesolution['operation'][l]='*';
                savesolution['resultat'][l]=plaque[l-1][0];
                s = compte(l-1);
                if (s != '')
                    return s;

                if(plaque1>=plaque2)
                {
                    plaque[l-1][0]=plaque1-plaque2;
                    if(plaque[l-1][0])
                    {
                        savesolution['operation'][l]='-';
                        savesolution['resultat'][l]=plaque[l-1][0];
                        s = compte(l-1);
                        if (s != '')
                            return s;
                    }
                    r = plaque1 % plaque2;
                    if(!r)
                    {
                        plaque[l-1][0] = plaque1 / plaque2;
                        savesolution['operation'][l]='/';
                        savesolution['resultat'][l]=plaque[l-1][0];
                        s = compte(l-1);
                        if (s != '')
                            return s;
                    }
                }
                else
                {
                    plaque[l-1][0] = plaque2 - plaque1;
                    savesolution['valeur1'][l] = plaque2;
                    savesolution['operation'][l] = '-';
                    savesolution['valeur2'][l]=plaque1;
                    savesolution['resultat'][l]=plaque[l-1][0];
                    s = compte(l-1);
                    if (s != '')
                        return s;

                    r = plaque2 % plaque1;
                    if(!r)
                    {
                        plaque[l-1][0] = plaque2 / plaque1;
                        savesolution['operation'][l] = '/';
                        savesolution['resultat'][l] = plaque[l-1][0];
                        s = compte(l-1);
                        if (s != '')
                            return s;
                    }
                }
            }
            else if(plaque1>=plaque2)
            {
                plaque[l-1][0] = plaque1 - plaque2;
                if(plaque[l-1][0])
                {
                    savesolution['operation'][l] = '-';
                    savesolution['resultat'][l] = plaque[l-1][0];
                    s = compte(l-1);
                    if (s != '')
                        return s;
                }
            }
            else
            {
                plaque[l-1][0]= plaque2 - plaque1;
                savesolution['valeur1'][l] = plaque2;
                savesolution['operation'][l] = '-';
                savesolution['valeur2'][l] = plaque1;
                savesolution['resultat'][l] = plaque[l-1][0];
                s = compte(l-1);
                if (s != '')
                    return s;
            }
        }
    }
}
