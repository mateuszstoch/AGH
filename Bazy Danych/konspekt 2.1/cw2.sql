--zadanie 1
select od.ProductID ,sum(od.Quantity) from [Order Details] od 
where od.ProductID < 3 group by od.ProductID 

--zadanie 2
select od.ProductID ,sum(od.Quantity) as ordered from [Order Details] od group by od.ProductID order by ordered desc

--zadanie 3
select od.OrderID,round(sum(od.Quantity * od.UnitPrice * (1-od.Discount ) ),2)from [Order Details] od 
group by od.OrderID having sum(od.Quantity) > 250